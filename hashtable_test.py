import unittest

from hypothesis import given, strategies as st

import hashtable


HASHTABLE_SIZE = 16


class TestHashtable(unittest.TestCase):
    def setUp(self):
        self.table = hashtable.create(HASHTABLE_SIZE)

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

    def test_put_get_failure1(self):
        for _ in range(100):
            key = "GE\x05Ê\U000fcd6d\U0002f7d3:"
            value = " 紿1¥&\x95\U00011528àñ\U00080a5a\U0004043döì\x8e"
            hashtable.put(self.table, key, value)
            actual = hashtable.get(self.table, key)
            self.assertEqual(value, actual)

    def test_put_get_failure2(self):
        for _ in range(100):
            key = "p\x04\U000a2cdc$\x06\U000e2134\x8f"
            value = "p\U000aa155©"
            hashtable.put(self.table, key, value)
            actual = hashtable.get(self.table, key)
            self.assertEqual(value, actual)


class TestHashtableWithHypothesis(unittest.TestCase):
    st_filter = lambda x: "\x00" not in x

    def setUp(self):
        self.table = hashtable.create(HASHTABLE_SIZE)

    def tearDown(self):
        hashtable.destroy(self.table)

    @given(st.dictionaries(st.text().filter(st_filter), st.text()))
    def test_put_get_with_random_data(self, input_dict):
        # Insert all key-value pairs from the input_dict into the hashtable
        for key, value in input_dict.items():
            hashtable.put(self.table, key, value)

        # Check that all inserted key-value pairs can be retrieved
        for key, value in input_dict.items():
            retrieved_value = hashtable.get(self.table, key)
            self.assertEqual(retrieved_value, value)

    @given(st.text().filter(st_filter))
    def test_non_existent_key_returns_none(self, key):
        self.assertIsNone(hashtable.get(self.table, key))


if __name__ == "__main__":
    unittest.main()
