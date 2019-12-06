/* ----==== CONSOLE.H ====---- */

#ifndef CONSOLE_H
#define CONSOLE_H

#include <string>
#include <fstream>
#include <vector>
#include <map>
#include "UTILITYCODE\singleton.h"
#include "UTILITYCODE\color.h"

/*---------------
---- DEFINES ----
---------------*/

#define console			Console::instance()	// used to access Console instance globally

typedef enum {
	CVAR_BOOL = 0,
	CVAR_CHAR,
	CVAR_UCHAR,
	CVAR_INT,
	CVAR_UINT,
	CVAR_FLOAT,
	CVAR_STRING
} tCVAR;

/*------------------
---- STRUCTURES ----
------------------*/


class CVar {
	public:
		tCVAR type;

		union {
			bool			b;
			char			c;
			unsigned char	uc;
			int				i;
			unsigned int	ui;
			float			f;
			std::string		*s;
		};

		void	operator= (bool _b)					{ if (type == CVAR_BOOL) b = _b; }
		void	operator= (unsigned char _uc)		{ if (type == CVAR_UCHAR) uc = _uc; }
		void	operator= (int _i)					{ if (type == CVAR_INT) i = _i; }
		void	operator= (float _f)				{ if (type == CVAR_FLOAT) f = _f; }
		void	operator= (const std::string &_s)	{ if (type == CVAR_STRING) *s = _s; }
		void	operator= (const char *_s)			{ if (type == CVAR_STRING) s->assign(_s); }

		explicit CVar(tCVAR _t)						{ type = _t; if (type == CVAR_STRING) s = new std::string(); }
		~CVar()										{ if (type == CVAR_STRING) delete s; }
};


class CHandler {	
	public:
		virtual void handleCommands(const char *, const std::vector<CVar*> &) const = 0;

		virtual ~CHandler() {}
};


class CCommand {
	friend class Console;

	private:
		std::string			argFormat;		// argument format sting
		unsigned char		numVars;		// the number of argument variables to take
		tCVAR				*types;			// list of argument variable types
		CHandler			*handler;		// Pointer to the command handler

	public:
		explicit CCommand(const std::string &_format, CHandler *_handler, unsigned char _numVars, tCVAR *_types);
		~CCommand();
};


struct CLine {
	std::string		text;
	Color3f			c;
};


class Console : public Singleton<Console> {
	private:

		std::vector<CLine>	line;				// list of lines
		std::string			scratchPad;			// line to enter commands
		std::string			priorLine;			// last line to be executed
		int					cursorPos;			// position of cursor on scratchpad
		bool				visible;			// console is visible or not

		std::map<std::string, CCommand*>	commands;		// list of console commands
		std::map<std::string, CVar*>		variables;		// list of console variables

		std::fstream			logFile;		// the console log file

	public:
		
		bool isVisible(void) const { return visible; }
		void switchVisible(void) { visible = !visible; }
		
		void addLine(const char *_text, ...);
		void addLine(const Color3f &col, const char *_text, ...);
		void addLineErr(const char *_text, ...);
		
		void processLine(const char *_command, ...);
		void input(void);
		void render(void);

		// CCommand interface
		void registerCommand(const std::string &_callName, const char *_format, CHandler *_handler,
							 unsigned char _numvars, ...);
		void unregisterCommand(const std::string &_callName);
		
		void handleBuiltInCommands(const std::string &_callName, const std::vector<CVar*> &_varList);
		void registerBuiltInCommands(void);
		void unregisterBuiltInCommands(void);

		// CVariable interface
		void registerVariable(const std::string &_callName, tCVAR _type);
		void unregisterVariable(const std::string &_callName);

		explicit Console();
		~Console();
};

#endif