/* ----==== CONSOLE.CPP ====---- */

#include <GL\glew.h>
#include <iostream>
#include <string>
#include "console.h"
#include "engine.h"
#include "options.h"
#include "UTILITYCODE\glfont.h"
#include "UTILITYCODE\keyboardmanager.h"


/*-----------------
---- FUNCTIONS ----
-----------------*/


int numtokens(const char *_string, const char *_delim)
{
	char tokenstring[80];
	char *token, *nextToken;
	strcpy_s(tokenstring,_string);
	int c = 0;

	token = strtok_s(tokenstring, _delim, &nextToken);
	while (token != 0) {
		c++;
		token = strtok_s(0, _delim, &nextToken);
	}

	return c;
}


////////// class CCommand //////////


CCommand::CCommand(const std::string &_format, CHandler *_handler, unsigned char _numVars, tCVAR *_types)
{
	argFormat = _format;

	numVars = _numVars;
	handler = _handler;

	types = new tCVAR[_numVars];
	for (int a = 0; a < _numVars; a++) types[a] = _types[a];
}


CCommand::~CCommand()
{
	delete [] types;
}


////////// class Console //////////


void Console::render(void)
{
	// glLoadIdentity() already called at end of PlayGame() before text is rendered

	const int fontSize = engine.font->getSize();

	glPushAttrib(GL_TRANSFORM_BIT | GL_VIEWPORT_BIT);

	glMatrixMode(GL_PROJECTION);									// Set our matrix to our projection matrix
	glPushMatrix();													// Push on a new matrix to work with
	glLoadIdentity();												// reset the matrix
	// Create a new viewport to draw into
	glViewport(0,gOptions.RESY-((gOptions.CONSOLELINES+2)*fontSize+20),gOptions.RESX,(gOptions.CONSOLELINES+2)*fontSize+20);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);
	glDisable(GL_TEXTURE_2D);
	glDepthMask(GL_FALSE);

	glEnable(GL_BLEND);
	glColor4f(0.2f,0.2f,0.2f,0.75f);
	glBegin(GL_QUADS);
		glVertex2f(-1,1);
		glVertex2f(-1,-1);
		glVertex2f(1,-1);
		glVertex2f(1,1);
	glEnd();
	glDisable(GL_BLEND);

	glColor3f(0,0,0.8f);
	glBegin(GL_LINES);
		glVertex2f(-1,-1);
		glVertex2f(1,-1);
	glEnd();

	glPopMatrix();										// Pop the projection matrix off the stack
	glPopAttrib();										// This restores our TRANSFORM and VIEWPORT attributes

	int r = gOptions.CONSOLELINES * fontSize;
	int count = 0;
	for (int l = line.size()-1; (l >= 0) && count < gOptions.CONSOLELINES; --l) {
		glColor3fv(line[l].c.v);
 		engine.font->print(fontSize, r, line[l].text.c_str());

		count++;
		r -= fontSize;
	}

	glColor3f(0,1,0);

	engine.font->print(fontSize,(gOptions.CONSOLELINES+2)*fontSize,"> %s", scratchPad.c_str());
	std::string cursorStr;
	for (int c = 0; c < cursorPos+2; ++c) cursorStr += " ";
	cursorStr += "_";
	engine.font->print(fontSize,(gOptions.CONSOLELINES+2)*fontSize, cursorStr.c_str());

	glDepthMask(GL_TRUE);
	if (!engine.cOptions.cameraView) glEnable(GL_FOG);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
}


void Console::addLine(const char *_text, ...)
{
	char tempstring[256];

	if (!_text || strlen(_text) > 255) return;

	va_list	arg;
	va_start(arg, _text);
	vsprintf_s(tempstring, _text, arg);
	va_end(arg);

	CLine newLine;
	newLine.text = tempstring;
	newLine.c.set(0.75f,0.75f,0.75f);

	line.push_back(newLine);

	// write line to the log file
	logFile << tempstring << std::endl;
}


void Console::addLine(const Color3f &col, const char *_text, ...)
{
	char tempstring[256];

	if (!_text || strlen(_text) > 255) return;

	va_list	arg;
	va_start(arg, _text);
	vsprintf_s(tempstring, _text, arg);
	va_end(arg);

	CLine newLine;
	newLine.text = tempstring;
	newLine.c = col;

	line.push_back(newLine);

	// write line to the log file
	logFile << tempstring << std::endl;
}


void Console::addLineErr(const char *_text, ...)
{
	char tempstring[256];

	if (!_text || strlen(_text) > 255) return;

	va_list	arg;
	va_start(arg, _text);
	vsprintf_s(tempstring, _text, arg);
	va_end(arg);

	CLine newLine;
	newLine.text = tempstring;
	newLine.c.set(1.0f,0.25f,0.25f);

	line.push_back(newLine);

	// write line to the log file
	logFile << tempstring << std::endl;
}


void Console::processLine(const char *_command, ...)
{
	// Handle arguements
	va_list	arg;
	char commandarg[256];
	commandarg[0] = '\0';

	va_start(arg, _command);
	vsprintf_s(commandarg, _command, arg);
	va_end(arg);

	char tokenstring[256];
	char *token, *nextToken;
	strcpy_s(tokenstring,commandarg);

	token = strtok_s(tokenstring," ", &nextToken);

	// find command
	std::map<std::string,CCommand*>::iterator c = commands.find(token);
	if (c != commands.end()) {
		CCommand *command = c->second;

		// re-copy the buffer to the token string
		strcpy_s(tokenstring,commandarg);

		// find start of var list
		char *spaceloc = strpbrk(tokenstring," ");
		int numVars = spaceloc == 0 ? 0 : numtokens(spaceloc+1,",");

		// go back to start of var list
		token = strtok_s(tokenstring," ",&nextToken);

		if (numVars == command->numVars) {
			std::vector<CVar*> currentVar(numVars);				

			for (int t = 0; t < numVars; ++t) {
				token = strtok_s(0,",",&nextToken);					
				currentVar[t] = new CVar(command->types[t]);

				switch (command->types[t]) {
					case CVAR_BOOL:							
						currentVar[t]->type = CVAR_BOOL;
						if (atoi(token) == 0) currentVar[t]->b = false;
						else currentVar[t]->b = true;
						break;

					case CVAR_UCHAR:
						currentVar[t]->type = CVAR_UCHAR;
						currentVar[t]->uc = (unsigned char)atoi(token);
						break;

					case CVAR_INT:
						currentVar[t]->type = CVAR_INT;
						currentVar[t]->i = atoi(token);
						break;

					case CVAR_FLOAT:
						currentVar[t]->type = CVAR_FLOAT;
						currentVar[t]->f = (float)atof(token);
						break;

					case CVAR_STRING:							
						currentVar[t]->type = CVAR_STRING;
						currentVar[t]->s->assign(token);
						break;
				}
			}

			if (command->handler) {	// if it is not a built-in function
				command->handler->handleCommands(c->first.c_str(), currentVar);
			} else {
				handleBuiltInCommands(c->first.c_str(), currentVar);
			}

			for (int v = 0; v < numVars; ++v) delete currentVar[v];
			currentVar.clear();
				
		} else {
			addLine("     incorrect syntax");
			addLine("          %s %s",c->first.c_str(),command->argFormat.c_str());
		}
	}
}


void Console::input(void)
{
	std::string tempStr;

	kb.appendAllInput(tempStr);
	scratchPad.insert(cursorPos,tempStr);
	cursorPos += tempStr.length();

	if (kb.keyTyped(VK_BACK) && cursorPos > 0) {
		--cursorPos;
		scratchPad.erase(cursorPos,1);
	}

	if (kb.keyDown(VK_DELETE) && cursorPos < scratchPad.length()) {
		scratchPad.erase(cursorPos,1);
	}

	if (kb.keyDown(VK_SHIFT) && kb.keyDown(VK_RIGHT) && cursorPos < priorLine.length() && cursorPos == scratchPad.length()) {
		scratchPad += priorLine[cursorPos];
		++cursorPos;
	} else if (kb.keyDown(VK_RIGHT) && cursorPos < scratchPad.length()) {
		++cursorPos;
	}

	if (kb.keyDown(VK_LEFT) && cursorPos > 0) {
		--cursorPos;
	}

	if (kb.keyTyped(VK_RETURN) && cursorPos > 0) {
		priorLine = scratchPad;
		addLine(Color3f(1,1,1),scratchPad.c_str());
				
		processLine(scratchPad.c_str());
		cursorPos = 0;
		scratchPad.clear();
	}
}


void Console::registerCommand(const std::string &_callName, const char *_format, CHandler *_handler,
							  unsigned char _numvars, ...)
{
	tCVAR types[256];

	va_list	arg;
	va_start(arg, _numvars);
	
	// parse arg list
	for (int a = 0; a < _numvars; ++a) types[a] = va_arg(arg, tCVAR);

	va_end(arg);

	// check list of commands for same callname
	std::map<std::string,CCommand*>::iterator c = commands.find(_callName);
	if (c != commands.end()) {
		addLineErr("register function failed: call name %s already in use", _callName.c_str());
		return;
	}

	// add new command to list
	CCommand *newCommand = new CCommand(_format,_handler,_numvars,types);
	commands.insert(std::pair<std::string,CCommand*>(_callName,newCommand));

	addLine(Color3f(0.9f,0.15f,0.9f), "function registered: %s", _callName.c_str());
}


void Console::unregisterCommand(const std::string &_callName)
{
	std::map<std::string,CCommand*>::iterator c = commands.find(_callName);
	if (c != commands.end()) {
		delete c->second;
		commands.erase(c);

		addLine(Color3f(0.66f,0.8f,0.66f),"function unregistered: %s",_callName.c_str());
	} else {
		addLineErr("unregister failed, function %s not found", _callName.c_str());
	}	
}


void Console::handleBuiltInCommands(const std::string &_callName, const std::vector<CVar*> &_varList)
{
	// commands function
	if (_stricmp(_callName.c_str(), "commands") == 0) {
		for (std::map<std::string,CCommand*>::iterator c = commands.begin(); c != commands.end(); ++c) {
			addLine("     %s %s", c->first.c_str(), c->second->argFormat.c_str());
		}
		return;
	}

	if ((_stricmp(_callName.c_str(), "exit") == 0) || (_stricmp(_callName.c_str(), "quit") == 0)) {
		engine.exitEngine = true;
		return;
	}

	// convertRAW function
	if (_stricmp(_callName.c_str(), "convertRAW") == 0) {
		const char *inFilename = _varList[0]->s->c_str();
		const char *outFilename = _varList[1]->s->c_str();
		const int tersize = _varList[2]->i;

		FILE *rawIn;
		if (fopen_s(&rawIn, inFilename,"rb") != 0) {
			addLine("     file \"%s\" not found",inFilename);
			return;
		}

		FILE *hmpOut;
		if (fopen_s(&hmpOut, outFilename,"wb") != 0) {
			addLine("     could not create file \"%s\"",outFilename);
			return;
		}
	
		unsigned char *inBuffer = new unsigned char[tersize*tersize];
		
		fseek(rawIn,0,SEEK_SET);
		fread(inBuffer,1,tersize*tersize,rawIn);
		fseek(hmpOut,0,SEEK_SET);
		fwrite(&tersize,4,1,hmpOut);
		fwrite(inBuffer,1,tersize*tersize,hmpOut);
		
		delete [] inBuffer;

		fclose(rawIn);
		fclose(hmpOut);

		addLine("     file \"%s\" written with size %i",outFilename,tersize);

		return;
	}

}


void Console::registerBuiltInCommands(void)
{
	registerCommand("commands","()",NULL,0);
	registerCommand("exit","()",NULL,0);
	registerCommand("quit","()",NULL,0);
	registerCommand("convertraw","(string inputFile, string outputFile, int terrainSize)",NULL,3,CVAR_STRING,CVAR_STRING,CVAR_INT);
}


void Console::unregisterBuiltInCommands(void)
{
	unregisterCommand("commands");
	unregisterCommand("exit");
	unregisterCommand("quit");
	unregisterCommand("convertraw");
}


void Console::registerVariable(const std::string &_callName, tCVAR _type)
{
	// check list of variables for same callname
	std::map<std::string,CVar*>::iterator v = variables.find(_callName);
	if (v != variables.end()) {
		addLineErr("register variable failed: call name %s already in use", _callName.c_str());
		return;
	}

	// add new command to list
	CVar *newVar = new CVar(_type);
	variables.insert(std::pair<std::string,CVar*>(_callName,newVar));

	addLine(Color3f(0.9f,0.15f,0.9f), "variable registered: %s", _callName.c_str());
}


void Console::unregisterVariable(const std::string &_callName)
{
	std::map<std::string,CVar*>::iterator v = variables.find(_callName);
	if (v != variables.end()) {
		delete v->second;
		variables.erase(v);

		addLine(Color3f(0.66f,0.8f,0.66f),"variable unregistered: %s",_callName.c_str());
	} else {
		addLineErr("unregister failed, variable %s not found", _callName.c_str());
	}	
}


Console::Console() : Singleton<Console>(*this)
{
	line.reserve(30);
	scratchPad.clear();
	priorLine.clear();
	visible = false;
	cursorPos = 0;

	// open or create console.log and clear it
	logFile.open("console.log", std::ios_base::out | std::ios_base::trunc);
}


Console::~Console()
{
	// Check to see if all commands were unregistered before deleting the console
	unregisterBuiltInCommands();
	if (!commands.empty()) addLine("NOT ALL FUNCTIONS WERE UNREGISTERED");
	
	logFile.close();

	line.clear();

	for (std::map<std::string,CCommand*>::iterator c = commands.begin(); c != commands.end(); ++c) delete c->second;
	for (std::map<std::string,CVar*>::iterator v = variables.begin(); v != variables.end(); ++v) delete v->second;
	commands.clear();
	variables.clear();
}
