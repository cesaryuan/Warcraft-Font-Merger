#pragma once

#include <cstdint>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

namespace otfcc::logger {

enum type {
	log_type_error = 0,
	log_type_warning = 1,
	log_type_info = 2,
	log_type_progress = 3,
};

enum {
	log_vl_critical = 0,
	log_vl_important = 1,
	log_vl_notice = 2,
	log_vl_info = 5,
	log_vl_progress = 10,
};

constexpr const char *names[3] = {"[ERROR]", "[WARNING]", "[NOTE]"};

class base {
  public:
	base() = default;
	virtual ~base() = default;

	// add a level
	virtual void indent(const char *segment) = 0;
	virtual void indent(const std::string &segment) = 0;

	// remove a level
	virtual void dedent() = 0;

	// add a level, output a progress
	virtual void start(const char *segment) = 0;
	virtual void start(const std::string &segment) = 0;

	// remove a level, finishing a task
	virtual void finish() = 0;

	// log a data
	virtual void log(uint8_t verbosity, type type, const char *data) = 0;
	virtual void log(uint8_t verbosity, type type, const std::string &data) = 0;

	// remove a level
	virtual void set_verbosity(uint8_t verbosity) = 0;
};

class empty : public base {
  public:
	empty() = default;
	~empty() = default;

	void indent(const char *segment) override {}
	void indent(const std::string &segment) override {}

	void dedent() override {}

	void start(const char *segment) override {}
	void start(const std::string &segment) override {}

	void finish() override {}

	void log(uint8_t verbosity, type type, const char *data) override {}
	void log(uint8_t verbosity, type type, const std::string &data) override {}

	void set_verbosity(uint8_t verbosity) override {}
};

class ostream : public base {
  private:
	std::ostream &_stream;
	std::vector<std::string> _indents;
	uint16_t _level;
	uint16_t _level_last;
	uint8_t _verbosity_limit;

  public:
	ostream(std::ostream &stream) : _stream(stream) {}
	~ostream() = default;

	void indent(const char *segment) override {
		_level++;
		_indents.emplace_back(segment);
	}
	void indent(const std::string &segment) override {
		_level++;
		_indents.push_back(segment);
	}

	void dedent() override {
		if (!_level)
			return;
		_level--;
		_indents.pop_back();
		if (_level < _level_last)
			_level_last = _level;
	};

	void start(const char *segment) override {
		indent(segment);
		log(log_vl_progress + _level, log_type_progress, "Begin");
	};
	void start(const std::string &segment) override {
		indent(segment);
		log(log_vl_progress + _level, log_type_progress, "Begin");
	};

	void finish() override {
		log(log_vl_progress + _level, log_type_progress, "Finish");
		dedent();
	};

	void log(uint8_t verbosity, type type, const char *data) override {
		log(verbosity, type, std::string(data));
	};
	void log(uint8_t verbosity, type type, const std::string &data) override {
		std::string demand;
		for (uint16_t level = 0; level < _level; level++) {
			if (level < _level_last - 1) {
				demand += std::string(_indents[level].length(), ' ');
				if (level < _level_last - 2)
					demand += " | ";
				else
					demand += " |-";
			} else
				demand += _indents[level] + " : ";
		}
		if (type < sizeof names / sizeof names[0])
			demand += std::string(names[type]) + " " + data;
		else
			demand += data;
		if (verbosity <= _verbosity_limit) {
			_stream << demand;
			if (*demand.rbegin() != '\n')
				_stream << std::endl;
			_level_last = _level;
		}
	};

	void set_verbosity(uint8_t verbosity) override { _verbosity_limit = verbosity; };
};

} // namespace otfcc::logger
