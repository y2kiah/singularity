#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include <GL\glew.h>
#include <sstream>
#include <vector>
#include <string>
#include "UTILITYCODE\msgassert.h"
#include "UTILITYCODE\singleton.h"


/*---------------
---- DEFINES ----
---------------*/

#define shader		ShaderManager::instance()	// used to access the ShaderManager instance globally


/*------------------
---- STRUCTURES ----
------------------*/

class CHandler;
class ShaderMgr_CAccess;


class ShaderProgram {
	friend class ShaderMgr_CAccess;

	private:
		GLhandleARB		program, vShader, fShader;
		std::string		vFilename, fFilename;
		bool			loadError;

		bool			compileVShader(const char *code);
		bool			compileFShader(const char *code);
		bool			createProgram(void);
		void			outputInfoLog(const GLenum &object, std::ios_base::openmode mode) const;

		CHandler		*cHandler;

	public:

		bool				hadLoadError() const { return loadError; }
		const std::string &	getVFilename() const { return vFilename; }
		const std::string &	getFFilename() const { return fFilename; }
		const GLhandleARB &	getProgramID() const { return program; }
		std::string			toString() const;
		std::string			getCompileLogs() const;

		explicit ShaderProgram(const std::string &_vFilename, const std::string &_fFilename);
		~ShaderProgram();

};


class ShaderManager : public Singleton<ShaderManager> {
	friend class ShaderMgr_CAccess;

	private:
		///// Variables

		std::vector<ShaderProgram*>		programs;

	public:
		CHandler	*cHandler;

		///// Functions

		bool	isEmpty(void) const { return programs.empty(); }
		int		loadFromFile(const std::string &vFilename, const std::string &fFilename);
		
		const GLhandleARB &	getShaderID(int i) const {	msgAssert(i >= 0 && i < programs.size(), "shader program out of range");
														return programs[i]->getProgramID(); }
		///// Constructors / Destructor

		explicit ShaderManager();
		~ShaderManager();
};

#endif