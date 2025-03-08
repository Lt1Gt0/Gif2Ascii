/* TODO
 * - Flag chaining (short flags)
 *   At some point the user should be able to chain flags together assuming that none of them require any input afterwards
 *   so instead of typing 
 *     -a -b -c
 *   they could just type
 *     -abc
 *
 * - Proper help printout
 *   When the help flag is called, information about each flag that the parser contains should be printed
 *   as well as information about its shorthand and longhand form, plus its description and argument count
 */


#pragma once
#ifndef _SKUTIL_FLAG_PARSER_HPP
#define _SKUTIL_FLAG_PARSER_HPP

#include <vector>
#include <iostream>
#include <cstring>

namespace SKUTIL
{
	#ifndef SKUTIL_VECTOR
		template<typename T>
		using SK_VEC = std::vector<T>;
	#endif

	#ifndef SKUTIL_FLAG_TYPES
		using SHORT_FLAG = char;
		using LONG_FLAG = const char*;

		constexpr const char* SHORT_FLAG_DELIM {"-"};
		constexpr const char* LONG_FLAG_DELIM {"--"};

 		#define OPT [[maybe_unused]]
	#endif 

	#ifndef SKUTIL_FLAG_PARAM_FUNC
		typedef void (*skutils_flag_param_func_)(OPT int inputCount, OPT char** inputVals);
	#endif
	
	// This is the maximum size of the allowable input length that can be given to a flag
	// so if the program was run as follows
	// ./[program] -f [input] [input]
	//
	// Each of the [input] values cannot exceed 1024 in length
	#ifndef SKUTIL_FLAG_VALUE_MAX_BUF_SIZE
		constexpr int SKUTIL_FLAG_VALUE_MAX_BUF_SIZE {1024};
	#endif

	/*
	 * Remove any nullptr values from a char buffer
	 *
	 * @param initCount (int): initial count of values that [buf] contains
	 * @param buf (char**): buffer to check for nullptrs
	 * @param outBuf(char**): Output buffer
	 *
	 * @returns: NONE
	 */  
	inline void RemoveNullStrs(int initCount, char** buf, char*** outBuf)
	{
		auto nullptrCount = [&]() -> int {
			int count = 0;
			
			for (int i = 0; i < initCount; i++) {
				if (buf[i] == nullptr)
					count++;	
			}

			return count;
		};

		// Set the size of the output buffer to the [buf] size minus the amount of nullptrs found
		int outBufSize = initCount - nullptrCount();
		*outBuf = new char*[outBufSize];

		for (int idx = 0, c = 0; c < outBufSize; idx++) {
			if (buf[idx] != nullptr) {
				*outBuf[c] = buf[idx];
				c++;
			}
		}
	}

	struct Flag
	{
		public:
			SHORT_FLAG mShortIdentifier;
			LONG_FLAG mLongIdentifier;
			const char* mDescription;

			// The amount of expected values (or inputs) to be give to the flag delimited by a space
			// If the flag has [mExpectedValueCount] set to 2 for example a valid use of the flag is as follows
			//
			// ./[program] -f 1 2
			//
			// Where -f is the flag and it expects 2 values to follow it
			int mExpectedValueCount;

			OPT skutils_flag_param_func_ mFunction;
			OPT char** mInputValues = nullptr;

			bool operator==(const Flag& other)
			{
				if (mShortIdentifier == other.mShortIdentifier ||
					std::strcmp(mLongIdentifier, other.mLongIdentifier) == 0)
					return true;

				return false;
			}

			bool operator!=(const Flag& other)
			{
				if (mShortIdentifier != other.mShortIdentifier &&
					std::strcmp(mLongIdentifier, other.mLongIdentifier) != 0)
					return true;

				return false;
			}
	};

	#ifndef STRUTIL_NULL_FLAG
		constexpr Flag NULL_FLAG = {' ', "", "", 0, nullptr};
	#endif 

	class FlagParser
	{
		public:
			SK_VEC<Flag>* mAllFlags;
			SK_VEC<Flag>* mSetFlags;
			FlagParser(SK_VEC<Flag>* _flags)
			{
				mAllFlags = _flags;

				// Check if any flags passed were reserved
				CheckForReserved();

				// After all custom flags have been added
				// append the reserve flags to mAllFlags
				for (size_t flagIdx = 0; flagIdx < RESV.size(); flagIdx++) {
					mAllFlags->push_back(RESV[flagIdx]);
				}
			}

			void ParseFlags(int argc, char** argv) 
			{
				for (int i = 1; i < argc; i++) {
					Flag f = NULL_FLAG;
					if (std::strncmp(argv[i], LONG_FLAG_DELIM, 2) == 0) {
						char* arg = new char[sizeof(argv[i]) - 2];

						// Strip the delimiter
						size_t idx = 0;
						while (idx <= sizeof(arg)) {
							arg[idx] = argv[i][idx + 2];
							idx++;
						}

						FindFlag(NULL_FLAG.mShortIdentifier, arg, &f);
					} else if (std::strncmp(argv[i], SHORT_FLAG_DELIM, 1) == 0) {
						// Strip the delimiter
						SHORT_FLAG arg = (char)(*(argv[i] + 1));
						FindFlag(arg, NULL_FLAG.mLongIdentifier, &f);
					}

					// If the flag is not null call its function
					if (f == NULL_FLAG)
						continue;

					// Get the respective values the flag wants as inputs
					if (f.mExpectedValueCount > 0) {
						f.mInputValues = new char*[f.mExpectedValueCount];
						i++;

						// I know loop in loop is bad but I really just want this flag parser to work for now
						for (int valIdx = 0; valIdx < f.mExpectedValueCount; valIdx++) {
							f.mInputValues[valIdx] = new char[SKUTIL_FLAG_VALUE_MAX_BUF_SIZE]; 
							f.mInputValues[valIdx] = argv[i + valIdx];
						}

						// After adding the values to the flag, the counter index for the next flag has to be set
						i += f.mExpectedValueCount - 1;
					}

					// This DOES NOT care if the values are valid or not
					// if say the flag expects 2 inputs and the user only passed 1
					// the input it would assume is valid is the next flag which
					// WILL cause potential issues
					f.mFunction(f.mExpectedValueCount, f.mInputValues);
				}

				// FIXME
				// RemoveNullStrs is broken at the moment but does not effect
				// the runtime of the code in current test cases so I will leave it commented out
				// for the time being 
				//
				// char** tmp = nullptr;
			}

			~FlagParser()
			{
				
			}

		private:
            static void ShowHelp(OPT int inputCount, OPT char** inputVals)
            {
                std::cout << "TODO: unimplemented\n";
            }

			SK_VEC<Flag> RESV = SK_VEC<Flag> {
				{
					'h',
					"help",
					"Show help commands",
					0,
				    ShowHelp,
				},
			};

			// TODO: Optimize this method further
			// Does not currently remove the flag from the list if it was
			// marked as reserve flag yet
			void CheckForReserved()
			{
				if (mAllFlags->empty())
					return;

				bool remove;
				for (Flag res : RESV) {
					remove = false;
					for (size_t i = 0; i < mAllFlags->size(); i++) {
						if (res.mShortIdentifier == mAllFlags->at(i).mShortIdentifier ||
							res.mLongIdentifier == mAllFlags->at(i).mLongIdentifier) {
							std::cerr << "Flag Was already reserved: " << "Short -> " << res.mShortIdentifier << std::endl;
							remove = true;
							// index = i;
						}
					}

					if (remove) {
						// mFlags->erase(index);
					}
				}
			}

			void FindFlag(OPT SHORT_FLAG shorthand, OPT LONG_FLAG longhand, Flag* outFlag)
			{
				*outFlag = NULL_FLAG;
				if (shorthand == NULL_FLAG.mShortIdentifier && 
					std::strcmp(longhand, NULL_FLAG.mLongIdentifier) == 0)
					return;

				for (Flag f : *mAllFlags) {
					if (shorthand == f.mShortIdentifier ||
						strcmp(longhand, f.mLongIdentifier) == 0) {
						*outFlag = f;
						return;
					}
				}	
			}

			bool FlagAlreadySet(const Flag& flag)
			{
				for (Flag f : *mSetFlags) {
					if (f.mShortIdentifier == flag.mShortIdentifier ||
						f.mLongIdentifier == flag.mLongIdentifier) {
						return true;
					}
				}

				return false;
			}
	};
}

// Remove all pre-processor macros that don't need to be defined for other programs
#undef SKUTIL_VECTOR 
#undef SKUTIL_FLAG_TYPES

#endif // _SKUTIL_FLAG_PARSER_HPP
