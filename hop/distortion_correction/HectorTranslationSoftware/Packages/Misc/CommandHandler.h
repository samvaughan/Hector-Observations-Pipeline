
//                      C o m m n a d  H a n d l e r . h
//
//  Function:
//    Provides a command line program with a way of handling arguments.
//
//  Description:
//    This defines a CommandHandler class that coordinates instances of a
//    set of classes derived from a base CmdArg class, instances of which
//    represent command line arguments of different types. For example, an
//    argument that represents a filename is handled by a FileArg instance,
//    an argument that represents a floating point number is handled by a
//    RealArg instance. The various arguments operate under the control of
//    a CommandHandler instance, which deals with parsing of the command line
//    parameters passed to the program. The individual argument instances all
//    provide a GetValue() call that can be used to return the value of the
//    corresponding argument.
//
//    The advantage these classes provide is that they handle:
//    o A very flexible command line syntax, allowing arguments to be specified
//      by name, and allowing boolean arguments to have negated forms, eg
//      either 'list' or 'nolist'.
//    o Command line constructs like "argument_name = value" are supported.
//    o Arguments can be specified by name or by position in the command lineo
//    o Provided types include string,file,real, int and bool.
//    o Variants of these types can be easily implemented by inheriting from
//      these provided types.
//    o Default values for the arguments can be specified.
//    o Type-dependent syntax checking and clear error reporting.
//    o The ability to remember argument values from previous runs of the
//      program and use them as defaults.
//    o Support for controlling which values have to be specified on the
//      command line and which can be allowed to default.
//    o Prompting for argument values left off from the command line.
//    o Optional checking for the existence of files by FileArg instances.
//    o When prompting for file names, a FileArg supports filename completion
//      (using TAB and ^D characters).
//    o Files can be specified using environment variables.
//    o Numeric values can be checked against allowed limits.
//    o Numeric values can be specified as 'min' and 'max'.
//
//    The result is that quite a lot of work can be packaged up into one
//    GetValue() call, and it's all done for you.
//
//  Author: Keith SHortridge, K&V (Keith@Knave&Varlet.com.au)
//
//  History:
//     22nd Jan 2021. Original version. KS.
//     31st Jan 2021. Revised internal structure to use strings internally
//                    to hold all values, moving almost all the code in
//                    the individual inhereting classes into the base CmdArg
//                    class. Added filename completion for filenames. KS.

#ifndef __CmdHandler__
#define __CmdHandler__

#include <string>
#include <list>
#include <vector>

//  Forward declarations

class CmdHandler;

//  ----------------------------------------------------------------------------
//
//                             C m d  A r g
//
//  The CmdArg class is the base class for the various argument types, and
//  does most of the work for them. The CmdArg class treats all values as
//  strings internally, and most of the code needed for the derived argument
//  types is merely connected with converting between these string values and
//  the actual type that they represent. The derived classes are also
//  responsible for validity checking for their argument types, which can
//  include interpreting special forms of the value, such as 'max' or 'min'
//  for numeric values or expanding environment variables used in file names
//  as done by FileArg. Each derived argument class is also expected to provide
//  a GetValue() call that returns the value of the argument in the expected
//  type (int, double, string, etc), and to provide a constructor that allows
//  reset values and limit values to be specified as examples of the type in
//  question.
//
//  A set of objects derived from CmdArg work together under the control of
//  a CommandHandler. Normally a Command Handler parses the command line
//  arguments passed to a program, working with its set of CmdArg objects,
//  and then the program can call the GetValue() method for each CmdArg
//  object to get the various argument values. If running interactively,
//  a CmdArg-based class object can prompt the user for the value of any
//  argument not explicitly specified on the command line, presenting a
//  sensible default value, or it can fall back on some other value that
//  it has been given - a default value, a previous value, or a 'reset'
//  value. Aguments can be specified on the command line by name or by
//  position.
//
//  (Note that it is assumed that there can always be a precise and reversible
//  conversion from the native type of any argument to a string and back. This
//  is obviously true for arguments whose values are basically strings, like
//  StringArg and FileArg, and is also true for IntArg, but it is not
//  self-evidently true for a RealArg. The trick is to support an 'exact'
//  string form for each argument, which is not necessarily the formatted
//  representation printf() or cout() would produce - it is more like a
//  serialisation of the internal representation. For most argument types the
//  'exact' string form is just that normal formatted form, eg '123' for the
//  integer 123, but it does not have to be, and the string values CmdArg works
//  with internally are always these 'exact' values. An exception is the call
//  DisplayValue(), which turns an internal 'exact' string into a formatted
//  value a user can understand, and which is null for most argument types,
//  but not for a RealArg.)
//
//  A CmdArg juggles a number of possible values for each argument:
//
//  o A 'Set' value specified in a set of arguments parsed by the Command
//    Handler. Usually these are the command line arguments for the program.
//  o A 'Default' value explicitly set in a SetDefault() call. Note that the
//    value of the reset argument is fixed once the CmdArg is created, but
//    the value for the default argument may change depending on circumstances.
//    A program may get the value of a file argument, open the file, and
//    depending on the file contents, the default for a later numeric
//    argument may change.
//  o A 'Previous' value, which is the value last used for this argument.
//    This may have been in a previous invocation of the program, since the
//    Command Handler can save and restore argument values.
//  o A 'Reset' value supplied in the argument constructor. This is the
//    value of last resort, used if nothing else has been specified.
//
//  Generally, these are used in the order of priority shown above. If a
//  value is specified explicitly on the command line, this value is used.
//  If no value was specified explicitly, then the CmdArg will try to determine
//  a default value. It will use a default value supplied explicitly by
//  the program, if there was one (on the assumption that the program knows
//  this is a sensible value), or if no explicit default was supplied, it will
//  use the previous value, if one is known, and as a last resort it will fall
//  back on the reset value specified in the constructor. If the program is
//  running interactively, the CmdArg can prompt the user for any value
//  unspecified on the command line, indicating in the prompt the default
//  value it has picked. The user can accept that, or supply a new value. If
//  the user supplies a value, that is the one that is used, so long as it
//  is valid.
//
//  Some control over this can be exercised:
//  o If an argument is flagged (in the Flags argument passed to the
//    constructor) as 'REQUIRED', then a value must be provided by the user.
//    If no valid value was set (ie on the command line) then a 'required'
//    argument will always be prompted for, if possible. If it cannot be
//    prompted for, then the CmdArg will report an error.
//  o If a value is not specified on the command line, but a valid default
//    value is available (either an explicit default, the previous value or
//    the reset value) and the argument is not flagged as 'REQUIRED' the
//    CmdArg will usually use the default without prompting. The Command
//    Handler can force prompting for all unspecified arguments, usually
//    because 'prompt' was specified on the command line.
//  o A CommandHandler can force a 'reset' condition (usually because
//    'reset' was explicitly specified on the command line. In this case,
//    any previous value is forgotten, and in the absence of an explicitly
//    specified default, the CmdArg will fall back on the reset value for
//    any argument unspecified on the command line. (It will, of course,
//    prompt for the value if possible.)
//  o The Command Handler can turn off prompting for unspecified arguments,
//    usually because 'noprompt' was specified on the command line. The
//    user can make use of this if they know that all the defaults, even
//    for required arguments, are valid. (Usually the effect is to use the
//    previous values for all unspecified arguments.)

class CmdArg {
public:
   //  -------------------------------------------------------------------------
   //  These are the routines that can be called from the program that creates
   //  the command handler and the arguments. These are the routines common to
   //  all the arguments. Note that these do not include the GetValue() routine
   //  each argument is expected to provide, as that returns the value in the
   //  specific type for the argument. The derived classes provide that, and
   //  are expected to use the protected DetermineValue() call to do most of
   //  the work. DetermineValue() provides a string version of the value,
   //  and the derived classes turn that into their native tyep. Similarly,
   //  these routines do not include the SetDefault() call that can provide a
   //  native default value. Nothing does by CmdArg uses native argument types
   //  like int, bool, real - it works entirely in strings.
   //  -------------------------------------------------------------------------
   //  Constructor.
   //  Name    is name of argument
   //  Posn    is position (0 => not positional, positions start at 1)
   //  Flags   flags - a string of comma/space separated options
   //  Prompt  prompt used if value has to be prompted for
   //  Text    more detailed explanatory text (currently unused)
   CmdArg(CmdHandler& Handler,const std::string& Name,int Posn,
          const std::string& Flags, const std::string& Prompt,
          const std::string& Text);
   //  Destructor.
   virtual ~CmdArg();
   //  Can be called to modify the text associated with an argument
   void SetText (const std::string& Text);
   //  Can be called to modify the prompt associated with an argument
   void SetPrompt (const std::string& Prompt);
   //  Can be called to modify the position associated with an argument
   void SetPosn (int Posn);
   //  Returns the description of the latest error.
   std::string GetError(void);
   //  Returns the name of the argument.
   std::string GetName (void) { return I_Name; }
   //  Returns a description of the argument.
   std::string Description (void);
protected:
   //  -------------------------------------------------------------------------
   //  These are utility routines provided for the use of the Command Handler
   //  and the derived argument classes.
   //  -------------------------------------------------------------------------
   //  Tells the argument that the command handler is closing down
   void CloseHandler (CmdHandler* Handler);
   //  Clears the argument ready for a new parsing or command line values
   bool Clear (void);
   //  Tells the argument to ignore any existing values.
   void Reset (void);
   //  Returns a "name = value" string giving the value of the argument.
   std::string GetCurrentSpec (void);
   //  Sets a string version of the default value - used by SetDefault()
   void SetDefaultValue (const std::string& Default);
   //  Determine the current value for the argument - used by GetValue()
   bool DetermineValue (std::string* CurrentValue);
   //  Returns true if the argument can be specified by its name only.
   bool ValueOptional (void) { return I_Valopt; }
   bool Required (void) { return I_Flags & REQUIRED; }
   bool Active (void) { return I_IsActive; }
   bool IsSet (void) { return I_IsSet; }
   bool Hidden (void) { return I_Flags & HIDDEN; }
   bool Internal (void) { return I_Flags & INTERNAL; }
   bool Negatable (void) { return I_Flags & NEGATABLE; }
   bool ValReq (void) { return I_Flags & VALREQ; }
   bool ValOpt (void) { return I_Flags & VALOPT; }
   void SetValOpt (bool Valopt) { I_Valopt = Valopt; }
   void SetReset (const std::string& Reset) { I_Reset = Reset; }
   void SetListing (bool Listing = true);
   bool IsListing (void) { return I_Listing; }
   void SetPrompting (const std::string& OnOffDefault);
   bool CanPrompt (void);
   bool DontSave (void) { return I_Flags & NOSAVE; }
   bool IsFile (void) { return I_Flags & IS_FILE; }
   void SetFlag (unsigned long Flag) { I_Flags |= Flag; }
   void SetConstructError (const std::string& Text) { I_ConstructError = Text;}
   void SetErrorText (const std::string& Text) { I_ErrorText = Text;}
   std::vector<std::string> GetUnknownFlags (void);
   void SetUnknownFlags (const std::vector<std::string>& Flags);
   //  These are routines called by the base CmdArg() routines that may need
   //  to be overriden by the derived argument routines.
   virtual bool SetPrevious (const std::string& Value);
   virtual bool TellUser (const std::string& ReportString);
   virtual bool ValidValue (const std::string& Value);
   virtual bool AllowedValue (const std::string& Value);
   virtual std::string PromptUser (const std::string& Default);
   virtual std::string DisplayValue (const std::string& Value);
   virtual std::string FormatForRecord (const std::string& Value);
   virtual void ExpandValue (std::string* Value);
   virtual bool MatchName (const std::string& Name);
   virtual bool SetValue (const std::string& Name, const std::string& Value);
   virtual bool SetValue (const std::string& Name, bool Equals,
                    const std::string Value, bool GotValue, bool* UsedValue);
   virtual bool NegName (const std::string& Name);
   virtual bool PosName (const std::string& Name);
   virtual std::string UnspecifiedValue(void);
   virtual std::string NegateValue(const std::string& Value);
   virtual std::string Requirement (void);
   //  The Flag values coresponding to the strings that can be passed
   //  to the constructor in the Flags argument.
   static const unsigned long REQUIRED = 0x1;    // Must be specified
   static const unsigned long VALOPT = 0x2;      // Does not need a value
   static const unsigned long VALREQ = 0x4;      // Must have a value a value
   static const unsigned long HIDDEN = 0x8;      // Not normally visible to user
   static const unsigned long NOSAVE = 0x10;     // Value should not be saved
   static const unsigned long NEGATABLE = 0x20;  // Argument name can be negated
   static const unsigned long IS_FILE = 0x40;    // Value is a file name
   static const unsigned long INTERNAL = 0x80;   // Set only by command handler
   friend class CmdHandler;
private:
   //  Pointer to the CmdHandler this argument is working with.
   CmdHandler* I_Handler;
   //  Name of the Argument.
   std::string I_Name;
   //  Prompt associated with the argument.
   std::string I_Prompt;
   //  More detailed text associated with the argument (not yet supported).
   std::string I_Text;
   //  Position (from 1 up) for a positional argument. 0 => non-positional.
   long I_Posn;
   //  Flags set for the argument.
   unsigned long I_Flags;
   //  True if a previous value has been set - the value used last time.
   bool I_PreviousSet;
   //  String version of the previous value.
   std::string I_Previous;
   //  True if a value has been set by the command handler - from a command line
   bool I_IsSet;
   //  Value set by the command handler, as a string
   std::string I_SetValue;
   //  True if a default value has been set dynamically.
   bool I_DefaultSet;
   //  Default value, as a string - a default that can be set dynamically
   std::string I_Default;
   //  Reset value, as a string - a default set in the constructor.
   std::string I_Reset;
   //  The value returned by GetValue() - ie the current value.
   std::string I_Value;
   //  True if the argument is active - ie has not been explicitly disabled.
   bool I_IsActive;
   //  True if the argument does not necessarily require a value.
   bool I_Valopt;
   //  True if the argument is to report the value it supplies in GetValue().
   bool I_Listing;
   //  True if prompting is on, even for nominally hidden arguments.
   bool I_PromptOn;
   //  True if prompting is off for all arguments.
   bool I_PromptOff;
   //  Describes any error that occurred during execution of the constructor.
   std::string I_ConstructError;
   //  Description of the last error associated with the argument.
   std::string I_ErrorText;
   //  Contains all the unrecognised items passed to the constructor in Flags.
   std::vector<std::string> I_UnknownFlags;
};

class StringArg : public CmdArg {
public:
   StringArg (CmdHandler& Handler,const std::string& Name,int Posn = 0,
      const std::string& Flags = "",const std::string& Reset = "",
      const std::string& Prompt = "",const std::string& Text = "");
   virtual ~StringArg();
   virtual std::string GetValue (bool* OK,std::string* Error);
   virtual void SetDefault (const std::string& Default);
};

class FileArg : public StringArg {
public:
   FileArg (CmdHandler& Handler,const std::string& Name,int Posn = 0,
      const std::string& Flags = "",const std::string& Reset = "",
      const std::string& Prompt = "",const std::string& Text = "");
   virtual ~FileArg();
protected:
   virtual bool FileExists (const std::string& FileName);
   virtual bool AllowedValue (const std::string& Value);
   virtual void ExpandValue (std::string* Value);
   virtual std::string Requirement (void);
private:
   bool NullOk (void) { return I_FileFlags & NULL_OK; }
   bool MustExist (void) { return I_FileFlags & MUST_EXIST; }
   static const unsigned long MUST_EXIST = 0x10; // File must exist already
   static const unsigned long NULL_OK = 0x100;   // Filename can be null
   unsigned long I_FileFlags;
   void ExpandTildePath (std::string* Path);
   std::vector<std::string> I_ResetValues;
};

class BoolArg : public CmdArg {
public:
   BoolArg (CmdHandler& Handler,const std::string& Name,int Posn = 0,
      const std::string& Flags = "",bool Reset = false,
      const std::string& Prompt = "",const std::string& Text = "");
   virtual ~BoolArg();
   virtual bool GetValue (bool* OK,std::string* Error);
   void SetDefault (bool Default);
protected:
   virtual bool ValidValue (const std::string& Value);
   virtual bool AllowedValue (const std::string& Value);
   virtual std::string NegateValue(const std::string& Value);
   virtual std::string UnspecifiedValue(void);
   virtual std::string Requirement (void);
private:
   bool CheckValidValue (const std::string& Value, bool* BoolValue);
};

class IntArg : public CmdArg {
public:
   IntArg (CmdHandler& Handler,const std::string& Name,int Posn = 0,
      const std::string& Flags = "",long Reset = 0,long Min = 0, long Max = 0,
      const std::string& Prompt = "",const std::string& Text = "");
   virtual ~IntArg();
   virtual long GetValue (bool* OK,std::string* Error);
   void SetDefault (long Default);
   virtual void SetRange (long Min, long Max);
protected:
   virtual bool ValidValue (const std::string& Value);
   virtual bool AllowedValue (const std::string& Value);
   virtual void ExpandValue (std::string* Value);
   virtual std::string Requirement (void);
private:
   bool CheckValidValue (const std::string& Value, long* IntValue);
   std::string FormatInt (long Value);
   long I_Min;
   long I_Max;
};

class RealArg : public CmdArg {
public:
   RealArg (CmdHandler& Handler,const std::string& Name,int Posn = 0,
      const std::string& Flags = "",double Reset = 0.0,double Min = 0.0,
      double Max = 0.0, const std::string& Prompt = "",
      const std::string& Text = "");
   virtual ~RealArg();
   virtual double GetValue (bool* OK,std::string* Error);
   void SetDefault (double Default);
   virtual void SetRange (double Min, double Max);
protected:
   virtual bool ValidValue (const std::string& Value);
   virtual bool AllowedValue (const std::string& Value);
   virtual void ExpandValue (std::string* Value);
   virtual std::string Requirement (void);
   virtual std::string DisplayValue (const std::string& Value);
private:
   bool CheckValidValue (const std::string& Value, double* RealValue);
   std::string FormatReal (double Value);
   std::string FormatRealExact (double Value);
   bool ReadRealExact (const std::string& Value,double* RealValue);
   bool HexCharChar2Char (char Ch1,char Ch2,unsigned char* Ch);
   double I_Min;
   double I_Max;
};

class CmdInteractor {
public:
   CmdInteractor (void);
   ~CmdInteractor();
   void SetInteractive (bool CanInteract = true);
   bool IsInteractive (void);
   virtual bool Write (const std::string& Text);
   virtual bool WriteLn (const std::string& Text = "");
   virtual bool ReadLn (std::string& Text,
                     const std::string& Prompt = "",bool IsFile = false);
   std::string GetError (void);
private:
   bool I_IsInteractive;
   std::string I_ErrorText;
};

class CmdHandler {
public:
   CmdHandler (const std::string& Program);
   ~CmdHandler();
   bool IsInteractive (void);
   bool ParseArgs (int Argc, char* Argv[]);
   bool ParseArgs (const std::vector<std::string>& Args);
   bool RemoveNamedArg (const std::string& Name);
   bool ReadPrevious (bool MustExist = false);
   bool SaveCurrent (void);
   std::string GetError (void);
protected:
   void DelArg (CmdArg* Arg);
   void AddArg (CmdArg* Arg);
   CmdInteractor* GetInteractor (void);
   friend class CmdArg;
private:
   std::string GetParameterFile(void);
   bool CheckSetup (void);
   bool I_ReadPrevious;
   bool I_Setup;
   std::string I_Program;
   std::string I_ErrorText;
   std::list<CmdArg*> I_CmdArgs;
   BoolArg* I_ResetArg;
   BoolArg* I_PromptArg;
   BoolArg* I_ListArg;
   CmdInteractor I_Interactor;
};

#endif
