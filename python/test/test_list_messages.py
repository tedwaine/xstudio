# SPDX-License-Identifier: Apache-2.0
import unittest

from xstudio.core import Link, Uuid, UuidActor, VectorString, VectorUuid, absolute_receive_timeout


class ListMessageTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.link = Link()
        cls.actor = cls.link.self()
        cls.uuid = Uuid("12345678-1234-1234-1234-123456789abc")

    def round_trip(self, values):
        self.link.send(self.actor, values)
        return self.link.dequeue_message_with_timeout(absolute_receive_timeout(1000))[0]

    def test_supported_lists(self):
        for values in ([1, -2], [1.5, -2.25], ["a", "b"], [self.uuid, self.uuid]):
            with self.subTest(values=values):
                self.assertEqual(list(self.round_trip(values)), values)

        result = self.round_trip([UuidActor(self.uuid, self.actor)])
        self.assertEqual(result[0].uuid, self.uuid)
        self.assertEqual(str(result[0].actor), str(self.actor))
        self.assertEqual(str(self.round_trip([self.actor])[0]), str(self.actor))

        self.assertEqual(len(self.round_trip(VectorUuid())), 0)

    def test_invalid_lists(self):
        for values in ([], [1, 2.0], [True], [object()], [2**40], [1e100]):
            with self.subTest(values=values):
                with self.assertRaises((RuntimeError, ValueError, OverflowError)):
                    self.link.send(self.actor, values)

    def test_uuid_strings(self):
        uuid_string = "12345678-1234-1234-1234-123456789abc"
        nil_string = "00000000-0000-0000-0000-000000000000"
        result = self.round_trip(uuid_string)
        self.assertIsInstance(result, Uuid)
        self.assertEqual(result, self.uuid)
        self.assertEqual(self.round_trip(nil_string), Uuid(nil_string))

        result = self.round_trip([uuid_string, nil_string])
        self.assertIsInstance(result, VectorUuid)
        self.assertEqual(list(result), [self.uuid, Uuid(nil_string)])

    def test_non_uuid_strings_are_preserved(self):
        uuid_string = "12345678-1234-1234-1234-123456789abc"
        invalid = uuid_string + "\0suffix"
        for value in ("not-a-uuid", invalid, uuid_string + "a"):
            with self.subTest(value=value):
                self.assertEqual(self.round_trip(value), value)
        for values in ([uuid_string, "not-a-uuid"], [invalid, uuid_string]):
            with self.subTest(values=values):
                result = self.round_trip(values)
                self.assertIsInstance(result, VectorString)
                self.assertEqual(list(result), values)
        result = self.round_trip(VectorString([uuid_string]))
        self.assertIsInstance(result, VectorString)
        self.assertEqual(list(result), [uuid_string])
