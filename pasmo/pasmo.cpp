// pasmo.cpp
// Revision 15-apr-2008


#include "pasmo.h"

#include "asm.h"



#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <chrono>  // for high_resolution_clock

using std::cout;
using std::cerr;
using std::endl;


namespace {


using std::string;
using std::vector;
using std::runtime_error;

    
#if MAC
    const string pasmoversion ("v0.1.3 (MAC)");
#else
    const string pasmoversion ("v0.1.3 (PC)");
#endif


class Usage { };

class NeedArgument : public runtime_error {
public:
	NeedArgument (const string & option) :
		runtime_error ("Option " + option + " requires argument")
	{ }
};


    
class InvalidOption : public runtime_error {
public:
	InvalidOption (const string & option) :
		runtime_error ("Invalid option: " + option)
	{ }
};

runtime_error CreateObjectError ("Error creating object file");
runtime_error SymbolFileError ("Error creating symbols file");
runtime_error PublicFileError ("Error creating public symbols file");


std::ostream * perr= & cerr;


const string opt1 ("-1");
const string opt8 ("-8");
const string optd ("-d");
const string optv ("-v");
const string optB ("-B");
const string optE ("-E");
const string optI ("-I");

const string opt86        ("--86");
const string optalocal    ("--alocal");
const string optamsdos    ("--amsdos");
const string optbin       ("--bin");
const string optbracket   ("--bracket");
const string optcdt       ("--cdt");
const string optcdtbas    ("--cdtbas");
const string optcmd       ("--cmd");
const string optequ       ("--equ");
const string opterr       ("--err");
const string opthex       ("--hex");
const string optmsx       ("--msx");
const string optname      ("--name");
const string optnocase    ("--nocase");
const string optpass3     ("--pass3");
const string optplus3dos  ("--plus3dos");
const string optprl       ("--prl");
const string optpublic    ("--public");
const string optcspecsymbols ("--cspecsymbols");
const string optsnasmerrors("--snasmerrors");
const string opttracedata ("--tracedata");
const string opttap       ("--tap");
const string opttapbas    ("--tapbas");
const string opttzx       ("--tzx");
const string opttzxbas    ("--tzxbas");
const string optw8080     ("--w8080");
const string optsna       ("--sna");


class Options {


public:
	Options (int argc, char * * argv);

	typedef void (Asm::* emitfunc_t) (std::ostream &);

	emitfunc_t getemit () const { return emitfunc; }
	bool redirerr () const { return redirecterr; }
	bool publiconly () const { return emitpublic; }
	bool getpass3 () const { return pass3; }
	bool getsnasmerrors() const { return snasmerrors; }
	bool getcspecsymbols() const { return cspecsymbols; }
	bool gettracedata() const { return tracedata; }
	string getfilein () const { return filein; }
	string getfileout () const { return fileout; }
	string getfilesymbol () const { return filesymbol; }
	string getfilepublic () const;
	string getheadername () const { return headername; }
	void apply (Asm & assembler) const;



private:
	emitfunc_t emitfunc;
	static const emitfunc_t emitdefault;

	bool verbose;
	bool emitpublic;
	bool cspecsymbols = false;
	Asm::DebugType debugtype;
	bool redirecterr;
	bool nocase;
	bool autolocal;
	bool bracketonly;
	bool warn8080;
	bool mode86;
	bool pass3;
	bool snasmerrors;
	bool tracedata;

	vector <string> includedir;
	vector <string> labelpredef;

	string filein;
	string fileout;
	string filesymbol;
	string filepublic;
	string headername;
};


const Options::emitfunc_t Options::emitdefault (& Asm::emitobject);


Options::Options (int argc, char * * argv) :
	emitfunc (emitdefault),
	verbose (false),
	emitpublic (false),
	debugtype (Asm::NoDebug),
	redirecterr (false),
	nocase (false),
	autolocal (false),
	bracketonly (false),
	warn8080 (false),
	mode86 (false),
	pass3 (false)
{
	int argpos;
    for (argpos= 1; argpos < argc; ++argpos)
	{
		const string arg (argv [argpos] );
		if (arg == optbin)
			emitfunc = &Asm::emitobject;
		else if (arg == opthex)
			emitfunc = &Asm::emithex;
		else if (arg == optprl)
			emitfunc = &Asm::emitprl;
		else if (arg == optcmd)
			emitfunc = &Asm::emitcmd;
		else if (arg == optpass3)
			pass3 = true;
		else if (arg == optplus3dos)
			emitfunc = &Asm::emitplus3dos;
		else if (arg == opttap)
			emitfunc = &Asm::emittap;
		else if (arg == opttzx)
			emitfunc = &Asm::emittzx;
		else if (arg == optcdt)
			emitfunc = &Asm::emitcdt;
		else if (arg == opttapbas)
			emitfunc = &Asm::emittapbas;
		else if (arg == opttzxbas)
			emitfunc = &Asm::emittzxbas;
		else if (arg == optcdtbas)
			emitfunc = &Asm::emitcdtbas;
		else if (arg == optamsdos)
			emitfunc = &Asm::emitamsdos;
		else if (arg == optmsx)
			emitfunc = &Asm::emitmsx;
		else if (arg == optsna)
			emitfunc = &Asm::emitsna;
		else if (arg == optpublic)
			emitpublic = true;
		else if (arg == optcspecsymbols)
			cspecsymbols = true;
		else if (arg == optsnasmerrors)
			snasmerrors = true;
		else if (arg == opttracedata)
			tracedata = true;
		else if (arg == optname)
		{
			++argpos;
			if (argpos >= argc)
				throw NeedArgument (optname);
			headername= argv [argpos];
		}
		else if (arg == optv)
			verbose= true;
		else if (arg == optd)
			debugtype= Asm::DebugSecondPass;
		else if (arg == opt1)
			debugtype= Asm::DebugAll;
		else if (arg == opterr)
			redirecterr= true;
		else if (arg == optnocase)
			nocase= true;
		else if (arg == optalocal)
			autolocal= true;
		else if (arg == optB)
			bracketonly= true;
		else if (arg == optbracket)
			bracketonly= true;
		else if (arg == opt8 || arg == optw8080)
			warn8080= true;
		else if (arg == opt86)
			mode86= true;
		else if (arg == optI)
		{
			++argpos;
			if (argpos >= argc)
				throw NeedArgument (optI);
			//a.addincludedir (argv [argpos] );
			includedir.push_back (argv [argpos] );
		}
		else if (arg == optE || arg == optequ)
		{
			++argpos;
			if (argpos >= argc)
				throw NeedArgument (arg);
			labelpredef.push_back (argv [argpos] );
		}
		else if (arg == "--")
		{
			++argpos;
			break;
		}
		else if (arg.substr (0, 1) == "-")
			throw InvalidOption (arg);
		else
			break;
	}

	// File parameters.

	if (argpos >= argc)
		throw Usage ();
	filein= argv [argpos];
	++argpos;
	if (argpos >= argc)
		throw Usage ();

	fileout= argv [argpos];
	++argpos;

	if (argpos < argc)
	{
		filesymbol= argv [argpos];
		++argpos;

		if (! emitpublic && argpos < argc)
		{
			filepublic= argv [argpos];
			++argpos;
		}

		if (argpos < argc)
			cerr << "WARNING: Extra arguments ignored" << endl;
	}

	if (headername.empty () )
		headername= fileout;
}

string Options::getfilepublic () const
{
	if (emitpublic)
		return filesymbol;
	else
		return filepublic;
}

void Options::apply (Asm & assembler) const
{
	assembler.setdebugtype (debugtype);

	if (verbose)
		assembler.verbose ();
	if (redirecterr)
		assembler.errtostdout ();
	if (nocase)
		assembler.caseinsensitive ();
	if (autolocal)
		assembler.autolocal ();
	if (bracketonly)
		assembler.bracketonly ();
	if (warn8080)
		assembler.warn8080 ();
	if (mode86)
		assembler.set86 ();
	if (pass3)
		assembler.setpass3 ();
	if (snasmerrors)
		assembler.snasmerrors();
	if (tracedata)
		assembler.tracedata();

	for (size_t i= 0; i < includedir.size (); ++i)
		assembler.addincludedir (includedir [i] );

	for (size_t i= 0; i < labelpredef.size (); ++i)
		assembler.addpredef (labelpredef [i] );

	assembler.setheadername (headername);
}


int doit (int argc, char * * argv)
{

	auto start = std::chrono::high_resolution_clock::now();

	// Process command line options.

	Options option (argc, argv);

	if (option.redirerr () )
		perr= & cout;

	// Assemble.

	Asm assembler;

	option.apply (assembler);

	assembler.loadfile (option.getfilein () ,option.getsnasmerrors());
	assembler.processfile ();


	// Generate ouptut file.

	std::ofstream out (option.getfileout ().c_str (),
		std::ios::out | std::ios::binary);
	if (! out.is_open () )
		throw CreateObjectError;

	(assembler.* option.getemit () ) (out);

	out.close ();

	// Generate symbol table and public symbol table if required.

	string filesymbol= option.getfilesymbol ();
	if (! option.publiconly () && ! filesymbol.empty () )
	{
		std::ofstream sout;
		std::streambuf * cout_buf= 0;
		if (filesymbol != "-")
		{
			sout.open (filesymbol.c_str () );
			if (! sout.is_open () )
				throw SymbolFileError;
			cout_buf= cout.rdbuf ();
			cout.rdbuf (sout.rdbuf () );
		}


		if (option.getcspecsymbols())
			assembler.dumpsymbolcspec(cout);
		else
			assembler.dumpsymbol (cout);
		if (cout_buf)
		{
			cout.rdbuf (cout_buf);
			sout.close ();
		}
	}

	string filepublic= option.getfilepublic ();
	if (! filepublic.empty () )
	{
		std::ofstream sout;
		std::streambuf * cout_buf= 0;
		if (filepublic != "-")
		{
			sout.open (filepublic.c_str () );
			if (! sout.is_open () )
				throw PublicFileError;
			cout_buf= cout.rdbuf ();
			cout.rdbuf (sout.rdbuf () );
		}
		assembler.dumppublic (cout);
		if (cout_buf)
		{
			cout.rdbuf (cout_buf);
			sout.close ();
		}
	}

	if (option.gettracedata())
	{
		assembler.dumpsymboltrace();

		assembler.closetracedata();
	}


	auto finish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = finish - start;
	std::cerr << "\nElapsed time: " << elapsed.count() << " s\n";


	return 0;
}


} // namespace


#if MAC
int Pmain (int argc, char * * argv)
#else
int main (int argc, char * * argv)
#endif
{
	// Just call doit and show possible errors.



	try
	{
		return doit (argc, argv);
	}
	catch (std::logic_error & e)
	{
		* perr << "ERROR: " << e.what () << endl <<
			"This error is unexpected, please "
			"send a bug report." << endl;
	}
	catch (std::exception & e)
	{
		* perr << "ERROR: " << e.what () << endl;
	}
	catch (Usage &)
	{
		cerr <<	"PasmoNext " << pasmoversion <<
			" (C) 2004-2005 Julian Albo\nModified by C Kirby\n"
			"Usage:\n\n"
			"\tpasmo [options] source object [symbol]\n\n"
			"See the README file for details.\n";
	}
	catch (...)
	{
		cerr << "ERROR: Unexpected exception.\n"
			"Please send a bug report.\n";
	}



	// Added to fix Debian bug report #394733
	return 1;
}

// End of pasmo.cpp



