////////////////////////////////////////////////////////////////////////////
//
//  MoCap File Importer
//  Copyright(c) Shun Cox (shuncox@gmail.com)
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
////////////////////////////////////////////////////////////////////////////

#include "idebug.h"
#include "imocapdatabvh.h"

using namespace imath;

//-----------------------------------------------------------------------------
// attach
//-----------------------------------------------------------------------------
int iMocapDataBvh::iTokenizerBvh::attach(istream *in)
{
	if (NULL == in) {
		ILOG4 ("Error: Invalid stream");
		return MC_INVALID_STREAM;
	}
	input = in;
	ILOG0 ("Success");
	return MC_SUCCESS;
}

//-----------------------------------------------------------------------------
// getWord
//-----------------------------------------------------------------------------
int iMocapDataBvh::iTokenizerBvh::getWord(string &oneword)
{
	// check input stream and skeleton
	if (NULL == input) {
		ILOG4 ("Error: Invalid stream");
		return MC_INVALID_STREAM;
	}
	
	if (iter == words.end()) {
		// get a line from file and parse it
		const string delimiter(spaceCharacters + "{}");
		string textLine;
		string::size_type prevPos, curPos;
		words.clear();
		prevPos = curPos = 0;

		// if EOF then return
		if (input->eof()) {
			ILOG0 ("End Of File");
			return MC_EOF;
		}

		// retrieve a line from stream
		getline(*input, textLine);
		// trim it
		textLine = trimString(textLine);

		while ((curPos = textLine.find_first_of(delimiter, curPos)) != string::npos) {
			// push the last word
			if (curPos != prevPos) words.push_back(textLine.substr(prevPos, curPos - prevPos));
			// trim leading spaces
//			curPos = textLine.find_first_not_of(spaceCharacters, curPos);
			while (curPos != string::npos && 
				spaceCharacters.find(textLine[curPos], 0) != string::npos
				) ++curPos;
			// is it a embrace or ':' ?
			if (curPos != string::npos && (
					textLine[curPos] == '{' ||
					textLine[curPos] == '}' ||
					textLine[curPos] == ':'
				)) {
				words.push_back(textLine.substr(curPos, 1));
				++curPos;
			}
			// save current position
			prevPos = curPos;
		}
		// process the final word
		if (prevPos != textLine.length()) words.push_back(textLine.substr(prevPos));

		iter = words.begin();
	}
	
	if (iter != words.end()) {
		// get a word from queue
		oneword = *iter;
		++iter;
		return MC_SUCCESS;
	}

	return MC_EOF;
}

//-----------------------------------------------------------------------------
// parse mocap data
//-----------------------------------------------------------------------------
int iMocapDataBvh::parsing(const string&  strJointNameSuffix)
{
	int result = MC_SUCCESS;
	// check input stream and skeleton
	if (NULL == input) {
		ILOG4 ("Error: Invalid stream");
		return MC_INVALID_STREAM;
	}
	if (NULL == skeleton) {
		ILOG4 ("Error: Invalid skeleton");
		return MC_INVALID_SKELETON;
	}
	// initialization
	enum MC_BVH_STAGE { MC_BVH_STAGE_NONE, MC_BVH_STAGE_SKELETON, MC_BVH_STAGE_MOTION };
	MC_BVH_STAGE stage = MC_BVH_STAGE_NONE;

	unsigned int frameCount = 0;	// quantity of frames
	float frameTime = 0;			// time of frame

	iTokenizerBvh tokenBvh(input);
	vector<iChannelLink> chanLinks;
	string word, jointName, parentName;

	//skeleton->clear();			// temporarily unsupported

	// tokenize the input stream
	while (MC_SUCCESS == result) {
		// get a word
		MC_GET_WORD;
		// convert the word to uppercase
		MC_TO_UPPER;
		switch (stage) {

		/////////////////////////////////////
		// The begining of BVH file
		case MC_BVH_STAGE_NONE:
			
			// should we goto the next stage ?
			if (!word.compare("HIERARCHY")) {
				ILOG1 (horizontalLine);
				ILOG1 (" Going to get the skeleton...");
				ILOG1 (horizontalLine);
				stage = MC_BVH_STAGE_SKELETON;
			}
			//cout << "|" << word;
			break;

		/////////////////////////////////////
		// Hierarchy section
		case MC_BVH_STAGE_SKELETON:
			
			// should we goto the next stage ?
			if (!word.compare("MOTION")) {
				ILOG1 (horizontalLine);
				ILOG1 (" Going to get the motion data...");
				ILOG1 (horizontalLine);
				stage = MC_BVH_STAGE_MOTION;
				break;
			}
			
			// root or joint is acceptable
			if (!word.compare("ROOT") || !word.compare("JOINT") || !word.compare("END")) {
				iVec offset;
				iVec rotation(0.0, 0.0, 0.0);		// useless in bvh files
				// save previous name into parentName
				parentName = jointName;
				jointName.clear();

				// Joint name section
				//--------------------

				// is it an End Site ?
				bool isEndSite = !word.compare("END");

				if (isEndSite) {
					MC_GET_WORD;
					MC_TO_UPPER;
					if (word.compare("SITE")) { result = MC_ILLEAGAL_DATA; break; }
					
					// search "_Effector" in parentName
					string::size_type pos = parentName.find(effectorPostfix, 0);
					if (string::npos != pos) {
						// multi 'End Site' : deprecative usage !!!
						ILOG3 ("Warning: Multi-'End Site' is deprecative!");
						// simply append an alphabeta character to it
						jointName = parentName + "X";
					} else {
						jointName = parentName + effectorPostfix;	// plus "_Effector"
					}
					// get a left embrace
					MC_GET_WORD;
				} else {
					// joint name
					MC_GET_WORD;
					// if it is not left embrace
					for (int i = 0; (i < maxWordsJointName) && word.compare("{"); ++i) {
						if (jointName.empty()) {
							jointName = word;
						} else {
							jointName.append(replacementOfSpace);
							jointName.append(word);
						}
						MC_GET_WORD;
					}
				}
				jointName+=strJointNameSuffix;
				// left embrace
				if (word.compare("{")) { result = MC_ILLEAGAL_DATA; break; }

				/*
				if (isEndSite) {
					MC_GET_WORD;
					MC_TO_UPPER;
					if (word.compare("SITE")) { result = MC_ILLEAGAL_DATA; break; }
					
					// search effector postfix in parentName
					string::size_type pos = parentName.find(effectorPostfix, 0);
					if (string::npos != pos) {
						// multi 'End Site' : deprecative usage !!!
						ILOG3 ("Warning: Multi-'End Site' is deprecative!");
						// simply append an alphabeta character to it
						jointName = parentName + "X";
					} else {
						jointName = parentName + "_Effector";
					}
				} else {
					// joint name
					MC_GET_WORD;
					jointName = word;
				}
				// left embrace
				MC_GET_WORD;
				if (word.compare("{")) { result = MC_ILLEAGAL_DATA; break; }
				*/

				//--------------------
				// End of section

				// joint offset
				MC_GET_WORD;
				MC_TO_UPPER;
				if (!word.compare("OFFSET")) {
					// three axies
					MC_GET_WORD;
					offset.x = fromString<float>(word);
					MC_GET_WORD;
					offset.y = fromString<float>(word);
					MC_GET_WORD;
					offset.z = fromString<float>(word);
				}
				if (!isEndSite) {
					// joint channels
					MC_GET_WORD;
					MC_TO_UPPER;
					if (!word.compare("CHANNELS")) {
						unsigned int i, ch;
						MC_GET_WORD;
						ch = fromString<unsigned int>(word);
						if (6 == ch) {
							// offset first
							iChannelLink lnk;
							lnk.jointName = jointName;
							lnk.typeOrder = "P";
							for (i = 0; i < 3; ++i) {
								MC_GET_WORD;
								MC_TO_UPPER;
								lnk.typeOrder += word[0];
							};
                            chanLinks.push_back(lnk);
							ch -= 3;
						}
						if (3 == ch) {
							// rotation only
							iChannelLink lnk;
							lnk.jointName = jointName;
							lnk.typeOrder = "R";
							for (i = 0; i < 3; ++i) {
								MC_GET_WORD;
								MC_TO_UPPER;
								lnk.typeOrder += word[0];
							};
                            chanLinks.push_back(lnk);
						} else {
							result = MC_ILLEAGAL_DATA;
							break;
						}
					} else {
						result = MC_ILLEAGAL_DATA;
						break;
					}
				}

				result = skeleton->addJoint(jointName, offset, rotation);
				if (MC_SUCCESS != result) {
					if (MC_DUP_JOINT_NAME == result) {
						ILOG4 ("Error: Joint duplicated");
					}
					break;
				}
				ILOG1 ("addJoint[ '" << jointName << "', " << offset << ", " << rotation << " ]");

			// right embrace
			} else if (!word.compare("}")) {
				if ((result = skeleton->goUp()) == MC_INVALID_JOINT) {
					ILOG1 ("Warning: Already the toppest");
					result = MC_SUCCESS;
				}
			}
			break;

		/////////////////////////////////////
		// Motion section
		case MC_BVH_STAGE_MOTION:

			// get quantity of frame
			if (!word.compare("FRAMES:")) {
				MC_GET_WORD;
				frameCount = fromString<unsigned int>(word);
			} else if (!word.compare("FRAMES")) {
				MC_GET_WORD;
				if (!word.compare(":")) {
					MC_GET_WORD;
				}
				frameCount = fromString<unsigned int>(word);
			} else {
				result = MC_ILLEAGAL_DATA;
				break;
			}
			ILOG2 ("Frames: " << frameCount);
			skeleton->setFrames(frameCount);

			// get frame time
			MC_GET_WORD;
			MC_TO_UPPER;
			if (!word.compare("FRAME")) {
				MC_GET_WORD;
				MC_TO_UPPER;
				if (!word.compare("TIME:")) {
					MC_GET_WORD;
					frameTime = fromString<float>(word);
				} else if (!word.compare("TIME")) {
					MC_GET_WORD;
					if (!word.compare(":")) {
						MC_GET_WORD;
					}
					frameTime = fromString<float>(word);
				} else {
					ILOG4 ("Error: Cannot find 'Time:'!");
					result = MC_ILLEAGAL_DATA;
					break;
				}
			} else {
				ILOG4 ("Error: Cannot find 'Frame Time:'!");
				result = MC_ILLEAGAL_DATA;
				break;
			}

			skeleton->setFrameTime(frameTime);
			ILOG2 ("Frame Time: " << skeleton->getFrameTime());

			//--------------------------------------------
			// Retrieving motion data
			//--------------------------------------------
			
#ifdef _DEBUG
			//================================================
			// channel listing
			//================================================
			{
				string s;
				for (vector<iChannelLink>::iterator iter = chanLinks.begin();
					iter != chanLinks.end(); ++iter) {
					s.append((*iter).jointName + "." + (*iter).typeOrder + ", ");
				}
				ILOG1("Channel = " << s);
			}
#endif
			//================================================
			// haha, get rotation order from the last channel!
			//================================================
			{
				string order(chanLinks.back().typeOrder);
				order.erase(0, 1);
				skeleton->setRotOrder(Rotation::getOrderFromString(order));
				ILOG1("RotationOrder = " << order << " (" << skeleton->getRotOrder() << ")");
			}
			//================================================
			
			for (unsigned int i = 0; i < frameCount; ++i) {
				ILOG1 (i << " " << horizontalLine);
				iSkeleton::iJoint *jot, *lastJoint = NULL;
				vector<iChannelLink>::iterator iter;
				for (iter = chanLinks.begin(); iter != chanLinks.end(); ++iter) {
					ILOG0 ((*iter).jointName);

					iSkeleton::iFrame frame;
					const string order((*iter).typeOrder);

					// make sure joint exist !!!
					jot = skeleton->getJoint((*iter).jointName);
					IASSERT(NULL != jot);

					if ('P' == order[0]) {
						for (unsigned int j = 1; j < 4; ++j) {
							MC_GET_WORD;
							switch(order[j]) {
							case 'X': frame.offset.x = fromString<float>(word); break;
							case 'Y': frame.offset.y = fromString<float>(word); break;
							case 'Z': frame.offset.z = fromString<float>(word); break;
							}
							ILOG0 ((*iter).jointName << " offset = " << frame.offset);
						}
					} else if ('R' == order[0]) {
						for (unsigned int j = 1; j < 4; ++j) {
							MC_GET_WORD;
							switch(order[j]) {
							case 'X': frame.rotation.x = fromString<float>(word); break;
							case 'Y': frame.rotation.y = fromString<float>(word); break;
							case 'Z': frame.rotation.z = fromString<float>(word); break;
							}
							ILOG0 ((*iter).jointName << " rotation = " << frame.rotation);
						}
					}
					if (jot == lastJoint) {
						if ('P' == order[0]) {
							((jot->motion).back()).offset = frame.offset;
						} else {
							((jot->motion).back()).rotation = frame.rotation;
						}
					} else {
						jot->motion.push_back(frame);
					}
					// transfer joint to next turn
					lastJoint = jot;

				}	// end of iter traverse
			}	// end of 'loop from 0 to frameCount
			
			break;
		}	// end of switch (stage)
	}	// end of while
	
	// don't worry, just reaching the end of file
	if (MC_EOF == result) {
		ILOG1 (horizontalLine);
		ILOG1 (" Aha! End Of File:)");
		ILOG1 (horizontalLine);
		result = MC_SUCCESS;
		if (MC_BVH_STAGE_SKELETON ==  stage) {
			ILOG3 ("Warning: No motion data is availible");
			result = MC_SUCCESS;
		} else if (MC_BVH_STAGE_NONE ==  stage) {
			ILOG4 ("Error: Illeage BVH file format");
			result = MC_ILLEAGAL_DATA;
		} else {
			/////////////////////////
			// TRICK!!!
			// get rid of base offset of root
			//
			if (skeleton->goTop() == MC_SUCCESS) {
				iVec zeroVec;
				skeleton->getJoint()->setOffset(zeroVec);
			}
			//
			/////////////////////////
			result = MC_SUCCESS;
		}
	}

	return result;
}
