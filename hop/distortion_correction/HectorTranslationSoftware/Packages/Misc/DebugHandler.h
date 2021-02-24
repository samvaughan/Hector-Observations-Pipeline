
//
//                          D e b u g  H a n d l e r
//
//   This implements a fairly basic way of controlling degug output from a
//   program. This imagines that a program consists of a number of sub-systems,
//   each of which supports varying levels of debug output. The idea is that
//   each subsystem has an instance of a DebugHandler. The constructor for the
//   DebugHandler is passed a string identifying the subsystem.
//
//   A call to the LevelsList() method supplies a comma-separated list of the
//   level names to be used. (And a ListLevels() call will return that list.)
//   LevelsList() should normally only be called once. It resets the Debug
//   Handler completely, with all levels assumed inactive.
//
//   A call to a SetLevels() method passes a comma-separated set of strings,
//   each of the form "SubSystem.level". Both the sub-system and level part of
//   the string can include wildcards ('*' for any number of characters, or
//   none, and '?' for a single character). If the subsystem matches that
//   supplied in the DebugHandler's constructor, the DebugHandler adds any
//   level that matches the level part of the string to its active list.
//
//   As the program runs, calls can be made to the Log() method, each supplying
//   a string giving a level and a text string. If the level matches any of its
//   active levels, the DebugHandler outputs the text string to standard output.
//   The Logf() method is like Log(), but accepts a format strign and a varying
//   number of arguments, like a traditional printf() call.
//
//   The idea is that the same level specification can be passed to each
//   DebugHandler, so "*.basic" could be used to turn on 'basic' debugging
//   for all subsystems. If the subsystem specification is missing, all
//   DebugHandlers will respond to the specification.
//
//   Levels can be unset using a call to UnsetLevels(), which is passed a
//   string identical to that for SetLevels() but which removes any matching
//   levels supported by the DebugHandler from the active list. (Debug code
//   could also use this to reduce excessive debug output, switching levels
//   on and off dynamically as execution procedes.)
//
//   A call to Active() returns true if a specified level is active, and
//   an be used to bypass a complete block of debug code if that is more
//   efficient than relying on the tests performed by each Log() call.
//
//  Author(s): Keith Shortridge, K&V  (Keith@KnaveAndVarlet.com.au)
//
//  History:
//     16th Jan 2021.  Original version. KS.

#ifndef __DebugHandler__
#define __DebugHandler__

#include "Wildcard.h"
#include "TcsUtil.h"

#include <string>
#include <vector>

#include <stdio.h>
#include <stdarg.h>

class DebugHandler {
public:

   DebugHandler (const std::string& SubSystem) {
      I_SubSystem = SubSystem;
   }
   
   ~DebugHandler () {}
   
   void LevelsList (const std::string& List) {
      TcsUtil::Tokenize(List,I_Levels,",");
      I_Flags.resize(I_Levels.size());
      for (int& Flag : I_Flags) { Flag = false; }
   }
   
   std::string ListLevels (void) {
      bool First = true;
      std::string Levels = "";
      for (std::string& Level : I_Levels) {
         if (!First) Levels = Levels + ",";
         First = false;
         Levels = Levels + Level;
      }
      return Levels;
   }
   
   void SetLevels (const std::string& Levels) {
      SetUnsetLevels (Levels,true);
   }
   
   void UnsetLevels (const std::string& Levels) {
      SetUnsetLevels (Levels,false);
   }
   
   bool Active (const std::string& Level) {
      bool Match = false;
      int NLevels = I_Levels.size();
      for (int I = 0; I < NLevels; I++) {
         if (TcsUtil::MatchCaseBlind(I_Levels[I].c_str(),Level.c_str())) {
            Match = I_Flags[I];
            break;
         }
      }
      return Match;
   }
   
   void Log (const std::string& Level,const std::string Text) {
      if (Active(Level)) {
         printf ("[%s.%s] %s\n",
                     I_SubSystem.c_str(),Level.c_str(),Text.c_str());
      }
   }
   
   void Logf (const std::string& Level,const char * const Format, ...) {
      if (Active(Level)) {
         char Message[1024];
         va_list Args;
         va_start (Args,Format);
         vsnprintf (Message,sizeof(Message),Format,Args);
         printf ("[%s.%s] %s\n",I_SubSystem.c_str(),Level.c_str(),Message);
      }
   }

private:

   //  SetUnsetLevels() does all the work for both SetLevels() and
   //  UnsetLevels(), the only difference being whether the matching levels
   //  are activated or deactivated.
   //  Levels a list of level specifiers, comma-separated, with each specifier
   //         a string that can include wildcard characters. This routine
   //         finds all matching levels and activates/deactivates them.
   //  Set    true if the matching levels are to be made active, false if
   //         they are to be deactivated.
   
   void SetUnsetLevels (const std::string& Levels, bool Set) {
      std::vector<std::string> Tokens;
      TcsUtil::Tokenize(Levels,Tokens,",");
      for (std::string Item : Tokens) {
         std::string SubSystem = "*";
         std::string Level = "*";
         size_t Dot = Item.find('.');
         if (Dot == std::string::npos) {
            Level = Item;
         } else {
            SubSystem = Item.substr(0,Dot);
            if (Dot < Item.size()) {
               Level = Item.substr(Dot + 1);
            }
         }
         if (WildcardMatchCaseBlind(SubSystem.c_str(),I_SubSystem.c_str())) {
            int NLevels = I_Levels.size();
            for (int I = 0; I < NLevels; I++) {
               if (WildcardMatchCaseBlind(Level.c_str(),I_Levels[I].c_str())) {
                  I_Flags[I] = Set;
               }
            }
         }
      }
   }

   //  The name of the current sub-system.
   std::string I_SubSystem;
   //  All the individual level names.
   std::vector<std::string> I_Levels;
   //  Flags for each level, true when the level is active.
   std::vector<int> I_Flags;
};

#endif

// ----------------------------------------------------------------------------------

/*                        P r o g r a m m i n g  N o t e s

 o  This really needs some more comments in the actual code, and exmaples
    migth help.
 
 o  I did play with using a map<string,bool> instead of the two vectors, one
    for the strings and one for the flags, but found it too awkward in the end,
    althogh it does feel like the obvious implementation. Maybe I'm just not
    as au fait with maps as I should be. I tried having I_Flags as a
    vector<bool> but this was made awkward by the speciailised implementation
    of vector<bool> which potentially packs up individual bools into bit
    patterns for efficiency. Storage efficiency isn't really important here.
 
*/
