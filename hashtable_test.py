import unittest

import hashtable


class TestHashtable(unittest.TestCase):
    def setUp(self):
        self.table = hashtable.create(16)

    def tearDown(self):
        hashtable.destroy(self.table)

    def test_put_get(self):
        # Test inserting a key-value pair and retrieving it
        hashtable.put(self.table, "key1", "value1")
        value = hashtable.get(self.table, "key1")
        self.assertEqual(value, "value1")

        # Test inserting another key-value pair and retrieving it
        hashtable.put(self.table, "key2", "value2")
        value = hashtable.get(self.table, "key2")
        self.assertEqual(value, "value2")

        # Test retrieving a non-existent key
        value = hashtable.get(self.table, "key3")

    def test_update_value(self):
        # Test updating the value for an existing key
        hashtable.put(self.table, "key1", "value1")
        hashtable.put(self.table, "key1", "new_value1")
        value = hashtable.get(self.table, "key1")
        self.assertEqual(value, "new_value1")


if __name__ == "__main__":
    unittest.main()
