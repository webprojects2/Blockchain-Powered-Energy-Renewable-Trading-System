import hashlib
import json
from datetime import datetime
from django.utils.timezone import now
from .models import BlockchainBlock


class Blockchain:
    def __init__(self):
        self.chain = self.load_chain()

    def load_chain(self):
        blocks = BlockchainBlock.objects.order_by("index")
        return list(blocks)

    def get_last_block(self):
        if not self.chain:
            return None
        return self.chain[-1]

    def create_genesis_block(self):
        if not self.chain:
            genesis_block = {
                "index": 0,
                "timestamp": now().isoformat(),
                "data": "Genesis Block",
                "previous_hash": "0" * 64
            }
            genesis_block["hash"] = self.compute_hash(genesis_block)
            return BlockchainBlock.objects.create(**genesis_block)

    def compute_hash(self, block_data: dict):
        """
        Computes SHA-256 hash of a block (excluding the hash itself).
        """
        block_copy = dict(block_data)
        block_copy.pop("hash", None)
        block_string = json.dumps(block_copy, sort_keys=True)
        return hashlib.sha256(block_string.encode()).hexdigest()

    def add_block(self, data: dict):
        """
        Adds a new block with given data.
        """
        last_block = self.get_last_block()
        index = last_block.index + 1 if last_block else 0
        previous_hash = last_block.hash if last_block else "0" * 64

        block = {
            "index": index,
            "timestamp": now().isoformat(),
            "data": data,
            "previous_hash": previous_hash
        }
        block["hash"] = self.compute_hash(block)

        block_obj = BlockchainBlock.objects.create(
            index=index,
            timestamp=block["timestamp"],
            data=json.dumps(data),
            previous_hash=previous_hash,
            hash=block["hash"]
        )
        self.chain.append(block_obj)
        return block_obj

    def is_valid_chain(self):
        """
        Validates the entire blockchain.
        """
        for i in range(1, len(self.chain)):
            current = self.chain[i]
            previous = self.chain[i - 1]

            # Validate hash
            expected_hash = self.compute_hash({
                "index": current.index,
                "timestamp": current.timestamp.isoformat(),
                "data": json.loads(current.data),
                "previous_hash": current.previous_hash
            })
            if current.hash != expected_hash:
                return False

            # Validate link
            if current.previous_hash != previous.hash:
                return False
        return True
