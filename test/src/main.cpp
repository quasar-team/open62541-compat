/*
 * main.cpp
 *
 *  Created on: Jun 11, 2015
 *      Author: bfarnham
 */

#include "gtest/gtest.h"

int main(int argc, char** argv)
{
	std::cout << "hello, world! This is your unit tests speaking" << std::endl;
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
