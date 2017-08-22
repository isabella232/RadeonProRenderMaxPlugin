/*********************************************************************************************************************************
* Radeon ProRender for 3ds Max plugin
* Copyright (c) 2017 AMD
* All Rights Reserved
*
* IES file parser
*********************************************************************************************************************************/

#pragma once

#include "utils\Utils.h"

// process data for IES light sources
class IESProcessor
{
public:
	class IESLightData;
	struct IESUpdateRequest;
	enum class ParseOrder;

	// parse IES file filename and fill lightData with data from this file
	// - in: filename: name filename of .ies file to be parsed
	// - out: lightData
	// - out: error message; in case Parse fails, information about error will be written to errorMessage
	// - returns: false for error, true if parse successfull 
	bool Parse (IESLightData& lightData, const char* filename, std::string& errorMessage) const;

	// save IES data in inner format in file (bin file?)
	// - in: lightData: struct with IES ligh parameters
	// - in: outfilename: name of file where data in internal representation should be saved
	// - returns: false for error, true if successfull 
	bool Save (const IESLightData& lightData, const char* outfilename) const;

	// load IES data from inner format
	// - in: name filename of file to be loaded
	// - out: lightData
	// - returns: false for error, true if parse successfull 
	bool Load (IESLightData& lightData, const char* filename) const;

	// change light data according to request, e.g. intensity is changed and so on
	// - in: req: struct with values to be updated in lightData
	// - in/out: lightData: struct with IES data that should be changed according to request req
	// - returns: false for error, true if successfull 
	bool Update (IESLightData& lightData, const IESUpdateRequest& req) const;

	// returns string representation of IES light source declaration
	// should be used for call to RPR
	// - returns string representing IES data
	std::string ToString (const IESLightData& lightData) const;

protected:
	// checks if filename is .ies file
	// - in: filename: name filename of file to be checked
	// - returns: true if file is ies file
	bool IsIESFile (const char* filename) const;

	// reads file and fills array tokens with numbers (as strings) read from the file
	// basically makes data more convinient to parse
	void GetTokensFromFile (std::vector<std::string>& tokens, std::ifstream& inputFile) const;

	// split line with several values separated by space(s) into array of strings each containing single value
	void SplitLine (std::vector<std::string>& tokens, const std::string& lineToParse) const;

	// fills lightData with data read from tokens
	// - returns false in case of parse failure
	bool ParseTokens (IESLightData& lightData, std::vector<std::string>& tokens, std::string& errorMessage) const;

	// auxilary function that hides interaction with enum
	// - returns first value of ParseOrder enum
	IESProcessor::ParseOrder FirstParseState (void) const;

	// fills corresponing to state lightData parameter with data read from value
	// value is supposed to string with one value, double or integer
	// - returns false in case of error
	bool ReadValue (IESLightData& lightData, IESProcessor::ParseOrder& state, const std::string& value) const;
};

// holds data for IES light source
class IESProcessor::IESLightData
{
public:
	IESLightData();

	// getters / setters
	// Should always be 1 according to Autodesk specification.
	DECLARE_VAR_AND_GETTERS(int, countLamps, CountLamps)

	// The initial rated lumens for the lamp used in the test or -1 if absolute photometry is used
	// and the intensity values do not depend on different lamp ratings. 
	DECLARE_VAR_AND_GETTERS(int, lumens, Lumens)

	// A multiplying factor for all the candela values in the file. 
	// This makes it possible to easily scale all the candela values in the file 
	// when the measuring device operates in unusual units�for example, 
	// when you obtain the photometric values from a catalog using a ruler on a goniometric diagram.
	// Normally the multiplying factor is 1.
	DECLARE_VAR_AND_GETTERS(int, multiplier, Multiplier)

	// The number of vertical (polar) angles in the photometric web. 
	DECLARE_VAR_AND_GETTERS(int, countVerticalAngles, CountVerticalAngles)

	// The number of horizontal (azimuth) angles in the photometric web. 
	DECLARE_VAR_AND_GETTERS(int, countHorizontalAngles, CountHorizontalAngles)

	// Should always be 1 according to Autodesk specification.
	// Can be 1, 2 or 3 according to IES specification.
	DECLARE_VAR_AND_GETTERS(int, photometricType, PhotometricType)

	// The type of unit used to measure the dimensions of the luminous opening.
	// Use 1 for feet or 2 for meters.
	DECLARE_VAR_AND_GETTERS(int, unit, Unit)

	// The width, length, and height of the luminous opening.
	DECLARE_VAR_AND_GETTERS(double, width, Width)
	DECLARE_VAR_AND_GETTERS(double, length, Length)
	DECLARE_VAR_AND_GETTERS(double, height, Height)

	// Multiplier representing difference between lab mesaurments and real world performance
	// Should always be 1 according to Autodesk specification.
	DECLARE_VAR_AND_GETTERS(int, ballast, Ballast)

	// Standard version
	// Should always be 1 according to Autodesk specification.
	DECLARE_VAR_AND_GETTERS(int, version, Version)

	// Power of light source
	// Should always be 0 according to Autodesk specification.
	DECLARE_VAR_AND_GETTERS(double, wattage, Wattage)

	// The set of vertical angles (aka polar angles), listed in increasing order. 
	// If the distribution lies completely in the bottom hemisphere, the first and last angles must be 0� and 90�, respectively.
	// If the distribution lies completely in the top hemisphere, the first and last angles must be 90� and 180�, respectively.
	// Otherwise, they must be 0� and 180�, respectively.
	DECLARE_VAR_AND_GETTERS(std::vector<double>, verticalAngles, VerticalAngles)

	// The set of horizontal angles (aka azimuth angles), listed in increasing order.
	// The first angle must be 0�.
	// The last angle determines the degree of lateral symmetry displayed by the intensity distribution.
	// If it is 0�, the distribution is axially symmetric.
	// If it is 90�, the distribution is symmetric in each quadrant.
	// If it is 180�, the distribution is symmetric about a vertical plane.
	// If it is greater than 180� and less than or equal to 360�, the distribution exhibits no lateral symmetries.
	// All other values are invalid. 
	DECLARE_VAR_AND_GETTERS(std::vector<double>, horizontalAngles, HorizontalAngles)

	// The set of candela values.
	// First all the candela values corresponding to the first horizontal angle
	// are listed, starting with the value corresponding to the smallest
	// vertical angle and moving up the associated vertical plane.
	// Then the candela values corresponding to the vertical plane
	// through the second horizontal angle are listed,
	// and so on until the last horizontal angle.
	// Each vertical slice of values must start on a new line.
	// Long lines may be broken between values as needed
	// by following the instructions given earlier. 
	DECLARE_VAR_AND_GETTERS(std::vector<double>, candelaValues, CandelaValues)

	// checks if struct holds correct data values
	// - returns false if data is corrupted
	bool IsValid (void) const;

	// deletes all data in this container
	void Clear (void);

	// the distribution is axially symmetric.
	bool IsAxiallySymmetric (void) const;

	// the distribution is symmetric in each quadrant.
	bool IsQuadrantSymmetric (void) const;

	// the distribution is symmetric about a vertical plane.
	bool IsPlaneSymmetric (void) const;

protected:
};

struct IESProcessor::IESUpdateRequest
{
	// NIY
};
