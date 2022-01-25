/*
 *	 Copyright (c) 2021 - Arclight Team
 *
 *	 This file is part of Arclight. All rights reserved.
 *
 *	 arcdebug.cpp
 */

#include "arcdebug.hpp"
#include "log.hpp"
#include "assert.hpp"


#ifndef ARC_FINAL_BUILD


ArcDebug::~ArcDebug() {
    flush();
}



void ArcDebug::dispatchToken(Token token) {

    switch(token) {

        case ArcSpace:
            buffer << ' ';
            break;
        
        case ArcEndl:
            flush();
            break;

        case ArcHex:
            buffer << std::hex;
            break;

        case ArcDec:
            buffer << std::dec;
            break;

        case ArcUpper:
            buffer << std::uppercase;
            break;

        case ArcNoUpper:
            buffer << std::nouppercase;
            break;

		case ArcForward:
			reversed = false;
			break;

		case ArcReversed:
			reversed = true;
			break;

        default:
            arc_force_assert("Invalid ArcDebug token");
            break;

    }

}


void ArcDebug::flush() noexcept {

    try {

		std::string str = buffer.str();
		buffer.str("");

		if(!str.empty() && str.back() == '\n') {
			str.resize(str.size() - 1);
		}

        Log::info("Debug", str);

    } catch(const std::exception& e) {
        arc_force_assert(std::string("Exception caught while flushing debug buffer: ") + e.what());
    }

}


#endif