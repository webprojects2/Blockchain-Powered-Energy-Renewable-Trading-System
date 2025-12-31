from decimal import Decimal
from django.utils import timezone
from .models import EnergyTransaction, UserProfile, BlockchainBlock
from .blockchain_utils import create_block
from django.db import transaction


class EnergySmartContractError(Exception):
    pass


class EnergySmartContract:
    """
    This class simulates a smart contract for energy trading.
    """

    def __init__(self, sender: UserProfile, receiver: UserProfile, energy_units: float, rate_per_unit: float):
        self.sender = sender
        self.receiver = receiver
        self.energy_units = Decimal(energy_units)
        self.rate_per_unit = Decimal(rate_per_unit)
        self.total_cost = self.energy_units * self.rate_per_unit
        self.timestamp = timezone.now()

    def validate_users(self):
        if self.sender == self.receiver:
            raise EnergySmartContractError("Sender and Receiver cannot be the same user.")

        if self.sender.energy_tokens < self.total_cost:
            raise EnergySmartContractError("Sender has insufficient energy tokens.")

    def execute(self):
        self.validate_users()

        with transaction.atomic():
            # Deduct tokens from sender
            self.sender.energy_tokens -= self.total_cost
            self.sender.total_energy_sold += self.energy_units
            self.sender.save()

            # Add tokens to receiver
            self.receiver.energy_tokens += self.total_cost
            self.receiver.total_energy_bought += self.energy_units
            self.receiver.save()

            # Record the transaction
            transaction_record = EnergyTransaction.objects.create(
                sender=self.sender,
                receiver=self.receiver,
                energy_transferred=self.energy_units,
                price_per_unit=self.rate_per_unit,
                timestamp=self.timestamp
            )

            # Generate blockchain block
            last_block = BlockchainBlock.objects.order_by("-created_at").first()
            previous_hash = last_block.hash if last_block else "0" * 64

            block_data = {
                "sender": self.sender.user.username,
                "receiver": self.receiver.user.username,
                "energy_units": str(self.energy_units),
                "rate_per_unit": str(self.rate_per_unit),
                "timestamp": self.timestamp.isoformat()
            }

            block = create_block(data=block_data, previous_hash=previous_hash)
            BlockchainBlock.objects.create(**block)

        return transaction_record
