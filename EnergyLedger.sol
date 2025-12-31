// SPDX-License-Identifier: MIT
pragma solidity ^0.8.19;

import "./EnergyToken.sol";
import "@openzeppelin/contracts/access/AccessControl.sol";

contract EnergyLedger is AccessControl {
    bytes32 public constant OPERATOR_ROLE = keccak256("OPERATOR_ROLE");
    EnergyToken public token;
    uint256 public lastSeq;

    struct Report {
        bytes32 meterIdHash; // hashed meter id for smaller storage
        uint256 timestamp;
        uint256 vrms; // scaled by 100 (e.g., 230.12 -> 23012)
        int256 activePowerW; // signed (positive import, positive generation depending convention)
        int256 energyWh;     // energy change in Wh, can be negative for export/import convention
        address wallet;      // optional on-chain wallet associated
    }

    // events
    event ReportRecorded(uint256 indexed seq, bytes32 meterIdHash, uint256 timestamp, int256 activePowerW, int256 energyWh, address wallet, string backendRef);
    event TokensMinted(address indexed to, uint256 amount, uint256 indexed seq);
    event TokensBurned(address indexed from, uint256 amount, uint256 indexed seq);

    mapping(uint256 => Report) public reports;

    constructor(address tokenAddress) {
        token = EnergyToken(tokenAddress);
        _setupRole(DEFAULT_ADMIN_ROLE, msg.sender);
        _setupRole(OPERATOR_ROLE, msg.sender);
        lastSeq = 0;
    }

    /// @notice operator records a meter reading. backend should be OPERATOR.
    /// @param meterIdHash hash of meter identifier
    /// @param timestamp unix seconds
    /// @param vrmsScaled scaled vrms (x100)
    /// @param activePowerW signed active power in W
    /// @param energyWh signed energy delta for window in Wh (multiply by 1 for Wh)
    /// @param wallet address optionally to credit/debit tokens
    /// @param backendRef off-chain reference (e.g., DB id or signature)
    function recordReport(
        bytes32 meterIdHash,
        uint256 timestamp,
        uint256 vrmsScaled,
        int256 activePowerW,
        int256 energyWh,
        address wallet,
        string calldata backendRef
    ) external onlyRole(OPERATOR_ROLE) returns (uint256) {
        lastSeq += 1;
        reports[lastSeq] = Report({
            meterIdHash: meterIdHash,
            timestamp: timestamp,
            vrms: vrmsScaled,
            activePowerW: activePowerW,
            energyWh: energyWh,
            wallet: wallet
        });

        emit ReportRecorded(lastSeq, meterIdHash, timestamp, activePowerW, energyWh, wallet, backendRef);

        // Optionally mint tokens for generation (if energyWh > 0 means exported energy)
        // Policy: if energyWh > 0 (export), mint tokens to wallet; if energyWh < 0 (import), optionally burn or debit.
        if (wallet != address(0)) {
            if (energyWh > 0) {
                uint256 mintAmount = uint256(energyWh); // 1 token == 1 Wh (choose scaling in deployment)
                token.mint(wallet, mintAmount);
                emit TokensMinted(wallet, mintAmount, lastSeq);
            } else if (energyWh < 0) {
                // If import (negative energyWh) we might require payment: burn tokens or leave balance management to off-chain settlement.
                // Example: burn tokens from wallet (requires token MINTER_ROLE to also have burn privileges)
                // token.burn(wallet, uint256(-energyWh));
                // emit TokensBurned(wallet, uint256(-energyWh), lastSeq);
            }
        }
        return lastSeq;
    }

    /// @notice convenience: operator can mint tokens manually
    function mintTokens(address to, uint256 amount) external onlyRole(OPERATOR_ROLE) {
        token.mint(to, amount);
        emit TokensMinted(to, amount, lastSeq);
    }
}
