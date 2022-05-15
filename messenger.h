#pragma once

#include <stdint.h>
#include <stdexcept>
#include <iterator>
#include <cassert>
#include <vector>
#include <string>

namespace messenger {

	struct msg_t
	{
		msg_t(const std::string& nm, const std::string& txt)
			: name(nm)
			, text(txt)
		{}

		std::string name;
		std::string text;
	};

	std::vector<uint8_t> make_buff(const msg_t& msg);

	msg_t parse_buff(std::vector<uint8_t>& buff);

}
