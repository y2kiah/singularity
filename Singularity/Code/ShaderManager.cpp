//	----==== SHADERMANAGER.CPP ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			4/05
//	Description:
//	----------------------------------------------------------------------------

#include <fstream>
#include <algorithm>
#include "shadermanager.h"
#include "shadermgr_console.h"


/*-----------------
---- FUNCTIONS ----
-----------------*/

////////// class ShaderProgram //////////

bool ShaderProgram::compileVShader(const char *code)
{
	vShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	if (vShader == 0) return false;

	glShaderSourceARB(vShader, 1, &code, 0);
	glCompileShaderARB(vShader);

	// Do compile error check and return
	int compiled = 0;
	glGetObjectParameterivARB(vShader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);
	if (compiled == 0) {
		console.addLineErr("     vertex shader had compile error(s)");
		outputInfoLog(vShader, std::ios_base::trunc);
		return false;
	} else {
		console.addLine("     vertex shader compiled successfully");
		outputInfoLog(vShader, std::ios_base::trunc);
		return true;
	}
}


bool ShaderProgram::compileFShader(const char *code)
{
	fShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	if (fShader == 0) return false;

	glShaderSourceARB(fShader, 1, &code, 0);
	glCompileShaderARB(fShader);

	// Do compile error check and return
	int compiled = 0;
	glGetObjectParameterivARB(fShader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);
	if (compiled == 0) {
		console.addLineErr("     fragment shader had compile error(s)");
		outputInfoLog(fShader, std::ios_base::ate);
		return false;
	} else {
		console.addLine("     fragment shader compiled successfully");
		outputInfoLog(fShader, std::ios_base::ate);
		return true;
	}
}


bool ShaderProgram::createProgram(void)
{
	program = glCreateProgramObjectARB();
	if (program == 0) return false;
		
	glAttachObjectARB(program, vShader);
	glAttachObjectARB(program, fShader);

	glLinkProgramARB(program);

	// Do link error check and return
	int linked = 0;
	glGetObjectParameterivARB(program, GL_OBJECT_LINK_STATUS_ARB, &linked);
	if (linked == 0) {
		console.addLineErr("     program had link error(s)");
		outputInfoLog(program, std::ios_base::ate);
		return false;
	} else {
		console.addLine("     program linked successfully");
		outputInfoLog(program, std::ios_base::ate);
		return true;
	}
}


void ShaderProgram::outputInfoLog(const GLhandleARB &object, std::ios_base::openmode mode) const
{
	std::ofstream outFile("infolog.txt", std::ios_base::out | mode);
	
	int logLength = 0;
	glGetObjectParameterivARB(object, GL_OBJECT_INFO_LOG_LENGTH_ARB , &logLength);
	if (logLength > 1) {
		char *logText = new char[logLength];
		glGetInfoLogARB(object, logLength, 0, logText);
   
		// write info log to console	
		outFile << logText;

		delete [] logText;
	}

	outFile.close();
}


std::string ShaderProgram::toString() const
{
	std::ostringstream returnStr;
	returnStr << "VERTEX SHADER: \"" << vFilename << "\"  -  FRAGMENT SHADER: \"" << fFilename << "\"";

	return returnStr.str();
}


std::string ShaderProgram::getCompileLogs() const
{
	std::ostringstream returnStr;

	int logLength = 0;
	glGetObjectParameterivARB(program, GL_OBJECT_INFO_LOG_LENGTH_ARB , &logLength);
	if (logLength > 1) {
		char *logText = new char[logLength];
		glGetInfoLogARB(program, logLength, 0, logText);
		returnStr << "PROGRAM:\n" << logText;

		delete [] logText;
	}
	
	logLength = 0;
	glGetObjectParameterivARB(vShader, GL_OBJECT_INFO_LOG_LENGTH_ARB , &logLength);
	if (logLength > 1) {
		char *logText = new char[logLength];
		glGetInfoLogARB(vShader, logLength, 0, logText);
		returnStr << "\nVERTEX SHADER:\n" << logText;

		delete [] logText;
	}
	
	logLength = 0;
	glGetObjectParameterivARB(fShader, GL_OBJECT_INFO_LOG_LENGTH_ARB , &logLength);
	if (logLength > 1) {
		char *logText = new char[logLength];
		glGetInfoLogARB(fShader, logLength, 0, logText);
		returnStr << "\nFRAGMENT SHADER:\n" << logText;

		delete [] logText;
	}

	return returnStr.str();
}


ShaderProgram::ShaderProgram(const std::string &_vFilename, const std::string &_fFilename)
{
	vFilename = _vFilename;
	fFilename = _fFilename;
	loadError = false;

	///// Vertex Program
	std::ifstream vFile(vFilename.c_str());
	if (!vFile) {
		loadError = true;
		console.addLineErr("ShaderManager: file \"%s\" not found", vFilename.c_str());
		return;
	}

	// convert extension to lower case for comparison
	std::string extension = vFilename.substr(vFilename.length()-5,5);
	std::transform(extension.begin(), extension.end(), extension.begin(), tolower);

	if (extension != ".vert") {
		loadError = true;
		console.addLineErr("ShaderManager: file \"%s\" extension not recognized as .vert", vFilename.c_str());
		vFile.close();
		return;
	}
	
	///// Fragment Program
	std::ifstream fFile(fFilename.c_str());
	if (!fFile) {
		loadError = true;
		console.addLineErr("ShaderManager: file \"%s\" not found", fFilename.c_str());
		vFile.close();
		return;
	}

	// convert extension to lower case for comparison
	extension = fFilename.substr(fFilename.length()-5,5);
	std::transform(extension.begin(), extension.end(), extension.begin(), tolower);

	if (extension != ".frag") {
		loadError = true;
		console.addLineErr("ShaderManager: file \"%s\" extension not recognized as .frag", fFilename.c_str());
		vFile.close();
		fFile.close();
		return;
	}

	///// Load shader code from files and compile

	std::ostringstream code;
	while (!vFile.eof()) {
		std::string strLine;
		std::getline(vFile, strLine);
		
		code << strLine << std::endl;
	}

	loadError = !compileVShader(code.str().c_str());

	std::ostringstream fCode;
	while (!fFile.eof()) {
		std::string strLine;
		std::getline(fFile, strLine);
		
		fCode << strLine << std::endl;
	}

	loadError = !compileFShader(fCode.str().c_str());

	///// Link progam
	loadError = !createProgram();

	///// Clean up
	vFile.close();
	fFile.close();
}


ShaderProgram::~ShaderProgram()
{
	glDeleteObjectARB(program);
	glDeleteObjectARB(vShader);
	glDeleteObjectARB(fShader);
}


////////// class ShaderManager //////////


int ShaderManager::loadFromFile(const std::string &vFilename, const std::string &fFilename)
{
	console.addLine(Color3f(1,1,1),"ShaderManager: loading program \"%s\", \"%s\"", vFilename.c_str(), fFilename.c_str());
	
	// Check if program files have already been loaded
	for (int s = 0; s < programs.size(); s++) {
		if (vFilename == programs[s]->getVFilename() && fFilename == programs[s]->getFFilename()) {
			console.addLineErr("     program already loaded");
			return s;	// return existing ID
		}
	}

	// Load the program
	ShaderProgram *newProgram = new ShaderProgram(vFilename, fFilename);
	if (newProgram->hadLoadError()) {
		console.addLineErr("program did not load successfully");
		delete newProgram;
		return -1;	// return -1 on error
	}

	programs.push_back(newProgram);

	console.addLine(Color3f(1,1,1),"program loaded successfully");

	return programs.size() - 1;	// return new program ID
}


ShaderManager::ShaderManager() : Singleton<ShaderManager>(*this)
{
	programs.reserve(2);

	cHandler = new ShaderMgr_CAccess();
}


ShaderManager::~ShaderManager()
{
	for (int s = 0; s < programs.size(); s++) {
		delete programs[s];
	}
	programs.clear();

	delete cHandler;
}