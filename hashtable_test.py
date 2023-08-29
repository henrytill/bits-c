import unittest

from hypothesis import given, strategies as st

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


class TestHashtableWithHypothesis(unittest.TestCase):
    @given(st.dictionaries(st.text().filter(lambda x: "\x00" not in x), st.text()))
    def test_put_get_with_random_data(self, input_dict):
        table = hashtable.create(16)

        # Insert all key-value pairs from the input_dict into the hashtable
        for key, value in input_dict.items():
            hashtable.put(table, key, value)

        # Check that all inserted key-value pairs can be retrieved
        for key, value in input_dict.items():
            retrieved_value = hashtable.get(table, key)
            self.assertEqual(retrieved_value, value)

        hashtable.destroy(table)

    @given(st.text().filter(lambda x: "\x00" not in x))
    def test_non_existent_key_returns_none(self, key):
        table = hashtable.create(16)
        self.assertIsNone(hashtable.get(table, key))
        hashtable.destroy(table)


if __name__ == "__main__":
    unittest.main()
