#define BOOST_TEST_MODULE mytests

#include <boost/test/included/unit_test.hpp>
#include "messenger.h"

BOOST_AUTO_TEST_CASE(myTestCase1) {
	std::cout << "Test case 1 (Long message)\n";
	std::string name    = "fakhriyor";
	std::string message = "aaaaaaaaaaaaaaabbbbbbbbbbbbbbbbccccccccccccccddddddddddddddddddddddeeeeeeeeeeeeeeeeeeeeeee";
	std::vector<uint8_t> testVec = messenger::make_buff(messenger::msg_t(name, message));
	messenger::msg_t     retObj  = messenger::parse_buff(testVec);
	BOOST_CHECK(retObj.text == message);
}

BOOST_AUTO_TEST_CASE(myTestCase2) {
	std::cout << "Test case 2 (Short message)\n";
	std::string name    = "fakhriyor";
	std::string message = "hello";
	std::vector<uint8_t> testVec = messenger::make_buff(messenger::msg_t(name, message));
	messenger::msg_t     retObj  = messenger::parse_buff(testVec);
	BOOST_CHECK(retObj.text == message);
}

BOOST_AUTO_TEST_CASE(myTestCase3) {
	std::cout << "Test case 3 (Empty name)\n";
	std::string name = "";
	std::string message = "hello";
	messenger::make_buff(messenger::msg_t(name, message));
}

BOOST_AUTO_TEST_CASE(myTestCase4) {
	std::cout << "Test case 4 (Empty message)\n";
	std::string name = "fakhriyor";
	std::string message = "";
	messenger::make_buff(messenger::msg_t(name, message));
}

BOOST_AUTO_TEST_CASE(myTestCase5) {
	std::cout << "Test case 5 (Modified message)\n";
	std::string name = "fakhriyor";
	std::string message = "hello";
	messenger::make_buff(messenger::msg_t(name, message));
	//						 'f'   'a'   'k'    'h'   'r'   'i'   'y'   'o'   'r'   'a'   'e'   'l'   'l'  'o'
	std::vector<uint8_t> modifiedMsg = { 0xb2, 0x52, 0x66, 0x61, 0x6b, 0x68, 0x72, 0x69, 0x79, 0x6f, 0x72, 0x61, 0x65, 0x6c, 0x6c, 0x6f };
	std::vector<uint8_t> testVec = messenger::make_buff(messenger::msg_t(name, message));
	messenger::msg_t     retObj = messenger::parse_buff(modifiedMsg);
	BOOST_CHECK(retObj.text != message);
}

BOOST_AUTO_TEST_CASE(myTestCase6) {
	std::cout << "Test case 6 (Modified name)\n";
	std::string name = "fakhriyor";
	std::string message = "hello";
	messenger::make_buff(messenger::msg_t(name, message));
	//						  'f'   'a'   'k'    'h'   'r'   'i'   'y'   'r'   'r'   'h'   'e'   'l'   'l'  'o'
	std::vector<uint8_t> modifiedNme = { 0xb2, 0x52, 0x66, 0x61, 0x6b, 0x68, 0x72, 0x69, 0x79, 0x72, 0x72, 0x68, 0x65, 0x6c, 0x6c, 0x6f };
	std::vector<uint8_t> testVec = messenger::make_buff(messenger::msg_t(name, message));
	messenger::msg_t     retObj = messenger::parse_buff(modifiedNme);
	BOOST_CHECK(retObj.name != name);
}

BOOST_AUTO_TEST_CASE(myTestCase7) {
	std::cout << "Test case 7 (Modified flag)\n";
	std::string name = "fakhriyor";
	std::string message = "hello";
	messenger::make_buff(messenger::msg_t(name, message));
	//				     0b111	   'f'   'a'   'k'    'h'   'r'   'i'   'y'   'o'   'r'   'h'   'e'   'l'   'l'  'o'
	std::vector<uint8_t> modifiedFlag = { 0xf2, 0x52, 0x66, 0x61, 0x6b, 0x68, 0x72, 0x69, 0x79, 0x6f, 0x72, 0x68, 0x65, 0x6c, 0x6c, 0x6f };
	std::vector<uint8_t> testVec = messenger::make_buff(messenger::msg_t(name, message));
	messenger::msg_t     retObj = messenger::parse_buff(modifiedFlag);
}
