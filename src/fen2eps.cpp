/* Fen2eps - A program for converting a FEN (Forsyth Edwards Notation)
*            string to an EPS (Encapsulated Postscript) file.
* Copyright (C) 2003-2010 by Dirk Baechle (dl9obn@darc.de)
*
* http://fen2eps.sourceforge.net
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the 
*
* Free Software Foundation, Inc.
* 675 Mass Ave
* Cambridge
* MA 02139
* USA
*
*/

/**
\file fen2eps.cpp
\author Dirk Baechle
\version 1.1
\date 2010-06-22
*/

/*------------------------------------------------------------- Includes */

#include <time.h>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>


using namespace std;

/*--------------------------------------------------------- Const values */

/** The total number of font symbols. */
const int ciFontSymbols = 50;
/** Names of the font symbols */
const char *pcSymbolNames[] = 
{
  "BS", "WPBS", "BPBS", "WNBS", "BNBS",
  "WBBS", "BBBS", "WRBS", "BRBS", "WQBS",
  "BQBS", "WKBS", "BKBS", "WS", "WPWS",
  "BPWS", "WNWS", "BNWS", "WBWS", "BBWS",
  "WRWS", "BRWS", "WQWS", "BQWS", "WKWS",
  "BKWS", "TF", "LF", "RF", "BF",
  "LFUC", "RFUC", "LFLC", "RFLC", "LFNA",
  "LFNB", "LFNC", "LFND", "LFNE", "LFNF",
  "LFNG", "LFNH", "BFNA", "BFNB", "BFNC",
  "BFND", "BFNE", "BFNF", "BFNG", "BFNH"
};

/** Marker type ``None'' */
const int mtNone = 0;
/** Marker type ``Begin'' */
const int mtBegin = 1;
/** Marker type ``End'' */
const int mtEnd = 2;

/*----------------------------------------------------- Global variables */

/** Struct that keeps all informations about the used
Postscript font. */
struct font_info
{
  /** Name of the font */
  string FontName;
  /** Version number of the font */
  string FontVersion;
  /** Release date of the font */
  string FontDate;
  /** Author/Creator of the font */
  string FontAuthor;
  /** EPS line width */
  double LineWidth;
  /** EPS translation to ULC of board in X direction */
  double TranslateX;
  /** EPS translation to ULC of board in Y direction */
  double TranslateY;
  /** EPS bounding box size in X direction */
  double BoundingBoxSizeX;
  /** EPS bounding box size in Y direction */
  double BoundingBoxSizeY;
  /** EPS scaling factor for the board */
  double ScaleFactor;
  /** EPS left margin */
  double LeftMargin;
  /** EPS right margin */
  double RightMargin;
  /** EPS top margin */
  double TopMargin;
  /** EPS bottom margin */
  double BottomMargin;
  /** Size of the total board */
  double BoardSize;
  /** Size of a board square */
  double SquareSize;
  /** Height of a board square above baseline */
  double SquareHeight;
  /** Depth of a board square below baseline */
  double SquareDepth;
  /** Height of top frame */
  double TopFrameHeight;
  /** Depth of top frame */
  double TopFrameDepth;
  /** Width of left frame without notation */
  double LeftFrameWidth;
  /** Height of left frame */
  double LeftFrameHeight;
  /** Depth of left frame */
  double LeftFrameDepth;
  /** Width of left frame with notation */
  double LeftNotationFrameWidth;
  /** Height of left frame with notation */
  double LeftNotationFrameHeight;
  /** Depth of left frame with notation */
  double LeftNotationFrameDepth;
  /** Widht of right frame */
  double RightFrameWidth;
  /** Height of right frame */
  double RightFrameHeight;
  /** Depth of right frame */
  double RightFrameDepth;
  /** Height of bottom frame */
  double BottomFrameHeight;
  /** Depth of bottom frame */
  double BottomFrameDepth;
  /** Height of bottom frame with notation */
  double BottomNotationFrameHeight;
  /** Depth of bottom frame with notation */
  double BottomNotationFrameDepth;
} fiFontInfo;

/** Current line number within the input file. */
unsigned int lineNumber = 0;
/** Prefix for automatically generated output files. */
string sPrefix = "";
/** Name of the font definition file. */
string sFontFile = "default.fed";
/** Is ``true'' if the output diagrams should be written into
separate files, each filename having the same prefix and a unique number,
``false'' else. */ 
bool bPrefixExport = false;
/** Number of the current output file for ``prefix'' mode. */
unsigned int fileNumber = 0;
/** String representation of number for current output file. */
string sFileNumber = "";
/** Is ``true'' if the board should be exported with notation,
``false'' else. */
bool bNotation = true;
/** Is ``true'' if the board should be displayed reverse,
``false'' else. */
bool bReverse = false;
/** Is ``true'' if the font definitions have been already 
exported once in ``prefix'' mode. */
bool bCompleteFontExported = false;
/** Array of boolean values that keeps information about
which symbol to export and which not....*/
bool pbSymbolExport[ciFontSymbols];
/*     0 = Black square */
/*  1-12 = PpNnBbRrQqKk on black square */
/*    13 = White square */
/* 14-25 = PpNnBbRrQqKk on white square */
/* 26-33 = Simple frame */
/* 34-41 = Left frame with digits */
/* 42-49 = Bottom frame with letters */
/** Array of integers that stores the current board position */
int piCurrentBoard[64];
/** Name of the current output file */
string sOutFile = "";

/*------------------------------------------------------------ Functions */

/** ``Simplifies'' the whitespaces (Space, Return, Tab) 
in the given string \a sLine, i.e. only single spaces
remain within the line and all leading and trailing
space characters are removed.
@param sLine String to be ``simplified'' 
*/
void simplifyWhiteSpace(string &sLine)
{
  // Is the string empty?
  if (sLine.length() == 0)
    return;

  // Replace all TABs with ``space''
  string::size_type pos = sLine.find("\t");
  while (pos != string::npos)
  {
    sLine.replace(pos, 1, " ");
    pos = sLine.find("\t");
  }

  // Replace all RETURNs with ``space''
  pos = sLine.find("\n");
  while (pos != string::npos)
  {
    sLine.replace(pos, 1, " ");
    pos = sLine.find("\n");
  }

  // Reduce to single spaces
  pos = sLine.find(" ");
  while (pos != string::npos)
  {
    // Remove additional spaces...
    sLine.erase(pos, sLine.find_first_not_of(" ", pos) - pos - 1);
    // Find next space
    pos = sLine.find(" ", ++pos);
  }

  // Remove leading space, if necessary...
  if (sLine.at(0) == ' ')
    sLine.erase(0, 1);

  // Remove trailing space, if necessary...
  if (sLine.at(sLine.length() - 1) == ' ')
    sLine.erase(0, 1);

}

/** Replaces all occurrences of the string \a toSearch by
\a toReplace within the string \a sLine.
@param sLine String that is to be changed
@param toSearch String to search for
@param toReplace String that is used as replacement
*/
void replaceAll(string &sLine, const string &toSearch, 
                const string &toReplace)
{
  // Is the string empty?
  if (sLine.length() == 0)
    return;

  // Search first occurrence
  string::size_type pos = sLine.find(toSearch);
  while (pos != string::npos)
  {
    sLine.replace(pos, toSearch.size(), toReplace);
    pos += toReplace.size();
    pos = sLine.find(toSearch, pos );
  }
}

/** Uses the character \a cSeparator in order to subdivide
the string \a sInput and returns the substring with number
\a sectionNumber in \a sSection. Substring numbers start
with `0', not `1'!
@warning Assumes that the string \a sInput does not start
with a separator!
@param sInput String to be divided into sections 
@param cSeparator The character that separates the single sections
@param sectionNumber Number of the wanted section
@param sSection The section string, which might be empty
*/
void stringSection(const string &sInput, 
                   const char cSeparator,
                   int sectionNumber,
                   string &sSection)
{
  string::size_type firstPos = 0;
  string::size_type secPos = 0;

  if (sectionNumber > 0)
  { 
    int cnt = 0;

    // Search start of substring
    while ((firstPos != string::npos) && 
           (cnt < sectionNumber))
    {
      cnt++;
      firstPos = sInput.find(cSeparator, firstPos + 1);
    } 

    // Nothing found, string too short?
    if (firstPos == string::npos)
    {
      sSection = "";
      return;
    }

    // Find end of substring
    secPos = sInput.find(cSeparator, firstPos + 1);
    // Assign substring
    if (secPos != string::npos)
      sSection = sInput.substr(firstPos + 1, secPos - firstPos - 1);
    else
      sSection = sInput.substr(firstPos + 1, secPos);
  }
  else
  {
    // Find end of substring
    secPos = sInput.find(cSeparator, firstPos + 1);
    // Assign substring
    sSection = sInput.substr(firstPos, secPos - firstPos);
  }

}

/** Scans the input file \a fIn for the next
``mark'' that starts with a \c BEGIN or \c END comment.
@param fIn Input file
@param blockName Name of the found block
@return Type of mark that was found (mtBegin, mtEnd or mtNone)
*/
int getNextMark(std::istream &fIn, string &blockName)
{
  if (fIn.eof())
    return mtNone;

  // Current input line
  string currentLine;

  getline(fIn, currentLine);
  while (!fIn.eof())
  {
    // New BEGIN mark found?
    if (currentLine.find("%BEGIN") == 0)
    {
      // Yes, so return name of the block
      stringSection(currentLine, ' ', 1, blockName);
      return mtBegin;
    }

    // New END mark found?
    if (currentLine.find("%END") == 0)
    {
      // Yes, so return name of the block
      stringSection(currentLine, ' ', 1, blockName);
      return mtEnd;
    }

    getline(fIn, currentLine);
  }

  return mtNone;
}

/** Skips the section named \a blockName within the
file \a fIn.
@param fIn Input file
@param blockName Name of the block
*/
void skipSection(std::istream &fIn, const string &blockName)
{
  // Found mark
  string sCurrentMark;
  // Marker type
  int markType;

  markType = getNextMark(fIn, sCurrentMark);
  while ((!fIn.eof()) &&
         ((markType != mtEnd) ||
         (sCurrentMark != blockName)))
    markType = getNextMark(fIn, sCurrentMark);
  
}

/** Writes the section named \a blockName from the file
\a fIn to the file \a fOut.
@param fIn Input file
@param fOut Output file
@param blockName Name of the block
@param bSimplify ``true'' if all lines should be ``whitespace-simplified'',
``false'' else (default)
*/
void writeSection(std::istream &fIn, std::ostream &fOut, 
                  const string &blockName, bool bSimplify = false)
{
  // Current line
  string sCurrentLine;
  // Current section name
  string sCurrentName;

  getline(fIn, sCurrentLine);
  while (!fIn.eof())
  {
    while (!fIn.eof() &&
           (sCurrentLine.find("%END") != 0))
    {
      if (bSimplify == true)
        simplifyWhiteSpace(sCurrentLine);
      fOut << sCurrentLine << endl;
      getline(fIn, sCurrentLine);
    }

    // Is the end of the correct section reached?
    stringSection(sCurrentLine, ' ', 1, sCurrentName);
    if (sCurrentName == blockName)
      return; // Yes

    // Continue...
    if (bSimplify == true)
      simplifyWhiteSpace(sCurrentLine);
    fOut << sCurrentLine << endl;
    getline(fIn, sCurrentLine);
    
  } 
}


/** Expands the FEN string in \a inputLine to the
position for the board \a piCurrentBoard.
@param inputLine Current input line
@return ``true'' if the conversion was successful and
the \a piCurrentBoard is valid, ``false'' else.
*/
bool expandFENString(string &inputLine)
{
  // Counters
  int row, col;

  // Reset export array for chess pieces (but not the frames)...
  for (row = 0; row < 26; row++)
    pbSymbolExport[row] = false;

  // Cut off everything after the first space...
  string::size_type pos = inputLine.find(' ');
  if (pos != string::npos)
    inputLine.erase(pos, string::npos);
  
  // Remove slashes...
  pos = inputLine.find('/');
  while (pos != string::npos)
  {
    inputLine.erase(pos, 1);
    pos = inputLine.find('/', pos);
  }

  // Expand digits 1-8 to equivalent number of empty squares
  replaceAll(inputLine, "1", " ");
  replaceAll(inputLine, "2", "  ");
  replaceAll(inputLine, "3", "   ");
  replaceAll(inputLine, "4", "    ");
  replaceAll(inputLine, "5", "     ");
  replaceAll(inputLine, "6", "      ");
  replaceAll(inputLine, "7", "       ");
  replaceAll(inputLine, "8", "        ");

  // Has the string the correct length now?
  if (inputLine.size() != 64)
    return false; // No

  // Current character
  char cCurrent;
  // Index for the symbol that has to be exported
  int iExport = 0;
  // Step through the expanded FEN string...
  for (row = 0; row < 8; row++)
  {
    for (col = 0; col < 8; col++)
    {
      if (((row*9 + col) % 2 ) == 0)
        iExport = 13; // White square
      else
        iExport = 0;  // Black square
      
      // Get current char
      cCurrent = inputLine.at(row*8+col);
      switch (cCurrent)
      {
        case 'P': // White pawn
                  iExport += 1;
                  break;
        case 'p': // Black pawn
                  iExport += 2;
                  break;
        case 'N': // White knight
                  iExport += 3;
                  break;
        case 'n': // Black knight
                  iExport += 4;
                  break;
        case 'B': // White bishop
                  iExport += 5;
                  break;
        case 'b': // Black bishop
                  iExport += 6;
                  break;
        case 'R': // White rook
                  iExport += 7;
                  break;
        case 'r': // Black rook
                  iExport += 8;
                  break;
        case 'Q': // White queen
                  iExport += 9;
                  break;
        case 'q': // Black queen
                  iExport += 10;
                  break;
        case 'K': // White king
                  iExport += 11;
                  break;
        case 'k': // Black king
                  iExport += 12;
                  break;
      }
      // Mark symbol for export...
      pbSymbolExport[iExport] = true;
      //...and store it in the current board.
      if (bReverse == true)
        piCurrentBoard[63-row*8-col] = iExport;
      else
        piCurrentBoard[row*8+col] = iExport;
    }
  }

  return true;
}

/** Checks whether the correct file header is present,
i.e. \a fIn is a Fen2eps font definition file.
@param fIn The input file
@return ``true'' if file header is correct, ``false'' else
*/
bool checkFileHeader(std::ifstream &fIn)
{
  string firstLine;

  getline(fIn, firstLine);

  if (firstLine.find("%Fen2eps Postscript font definition file") == 0)
    return true;
  else
    return false;
}



/** Reads the string \a s of the section \a sectionName 
from the info file \a fIn.
@param fIn The input file
@param sectionName Name of the section
@param s Read string
*/
void readString(std::ifstream &fIn, const string &sectionName, string &s)
{
  getline(fIn, s);

  // Is a string contained in the section?
  if (s.find("%END") == 0)
  {
    // No, so set string to ``empty''
    s = "";
    return;
  }

  // Skip to end of section...
  skipSection(fIn, sectionName);

}

/** Reads the number \a n of the section \a sectionName 
from the info file \a fIn.
@param fIn The input file
@param sectionName Name of the section
@param n Number value
*/
void readNumber(std::ifstream &fIn, const string &sectionName, double *n)
{
  // Current input line
  string currentLine;

  getline(fIn, currentLine);
  // Is a string contained in the section?
  if (currentLine.find("%END") == 0)
  {
    // No, so set number to ``zero''
    *n = 0.0;
    return;
  }

  // Convert string to number
  istringstream sNumber(currentLine);
  sNumber >> *n;
  if (*n < 0.0)
    *n *= -1.0;

  // Skip to end of section...
  skipSection(fIn, sectionName);

}

/** Reads the number \a n of the section \a sectionName 
from the info file \a fIn. The number string may also
contain one of the following unit specifications:
``cm'', ``mm'', ``in'' which are automatically
converted to Postscript units (1/72 inch).
@param fIn The input file
@param sectionName Name of the section
@param n Number value
*/
void readPSDimension(std::ifstream &fIn, const string &sectionName, double *n)
{
  // Current input line
  string currentLine;

  getline(fIn, currentLine);
  // Is a string contained in the section?
  if (currentLine.find("%END") == 0)
  {
    // No, so set number to ``zero''
    *n = 0.0;
    return;
  }

  // Convert string to number
  istringstream sNumber(currentLine);
  sNumber >> *n;
  if (*n < 0.0)
    *n *= -1.0;
  string sDimension = "";
  sNumber >> sDimension;

  // Dimension scaling
  if (sDimension == "cm")
    *n *= 72.0/2.54;
  if (sDimension == "mm")
    *n *= 7.2/2.54;
  if (sDimension == "in")
    *n *= 72.0;

  // Skip to end of section...
  skipSection(fIn, sectionName);

}

/** Reads the font infos from the given file \a fIn.
@param fIn The input file
*/
void readFontInfos(std::ifstream &fIn)
{
  // Found mark
  string sCurrentMark;
  // Marker type
  int markType;

  // Skip to ``FontInfo'' section
  markType = getNextMark(fIn, sCurrentMark);
  while ((!fIn.eof()) &&
         (markType != mtBegin) &&
         (sCurrentMark != "FontInfo"))
    markType = getNextMark(fIn, sCurrentMark);

  // No infos found?
  if (fIn.eof())
  {
    cerr << "Error: No ``FontInfo'' section found in " << sFontFile << "!" << endl;
    std::exit(1);
  }

  // Read info entries until the end of the ``FontInfo'' section
  markType = getNextMark(fIn, sCurrentMark);
  while ((!fIn.eof()) &&
         ((markType != mtEnd) ||
         (sCurrentMark != "FontInfo")))
  {
    // Is it the start of a section?
    if (markType == mtBegin)
    {
      // Process the found section
      if (sCurrentMark == "FontName")
        readString(fIn, sCurrentMark, fiFontInfo.FontName); 
      if (sCurrentMark == "FontVersion")
        readString(fIn, sCurrentMark, fiFontInfo.FontVersion); 
      if (sCurrentMark == "FontDate")
        readString(fIn, sCurrentMark, fiFontInfo.FontDate); 
      if (sCurrentMark == "FontAuthor")
        readString(fIn, sCurrentMark, fiFontInfo.FontAuthor); 
      
      if (sCurrentMark == "SquareSize")
        readNumber(fIn, sCurrentMark, &(fiFontInfo.SquareSize)); 
      if (sCurrentMark == "SquareHeight")
        readNumber(fIn, sCurrentMark, &(fiFontInfo.SquareHeight)); 
      if (sCurrentMark == "SquareDepth")
        readNumber(fIn, sCurrentMark, &(fiFontInfo.SquareDepth)); 
      if (sCurrentMark == "TopFrameHeight")
        readNumber(fIn, sCurrentMark, &(fiFontInfo.TopFrameHeight)); 
      if (sCurrentMark == "TopFrameDepth")
        readNumber(fIn, sCurrentMark, &(fiFontInfo.TopFrameDepth)); 
      if (sCurrentMark == "LeftFrameWidth")
        readNumber(fIn, sCurrentMark, &(fiFontInfo.LeftFrameWidth)); 
      if (sCurrentMark == "LeftFrameHeight")
        readNumber(fIn, sCurrentMark, &(fiFontInfo.LeftFrameHeight)); 
      if (sCurrentMark == "LeftFrameDepth")
        readNumber(fIn, sCurrentMark, &(fiFontInfo.LeftFrameDepth)); 
      if (sCurrentMark == "LeftNotationFrameWidth")
        readNumber(fIn, sCurrentMark, &(fiFontInfo.LeftNotationFrameWidth)); 
      if (sCurrentMark == "LeftNotationFrameHeight")
        readNumber(fIn, sCurrentMark, &(fiFontInfo.LeftNotationFrameHeight)); 
      if (sCurrentMark == "LeftNotationFrameDepth")
        readNumber(fIn, sCurrentMark, &(fiFontInfo.LeftNotationFrameDepth)); 
      if (sCurrentMark == "RightFrameWidth")
        readNumber(fIn, sCurrentMark, &(fiFontInfo.RightFrameWidth)); 
      if (sCurrentMark == "RightFrameHeight")
        readNumber(fIn, sCurrentMark, &(fiFontInfo.RightFrameHeight)); 
      if (sCurrentMark == "RightFrameDepth")
        readNumber(fIn, sCurrentMark, &(fiFontInfo.RightFrameDepth)); 
      if (sCurrentMark == "BottomFrameHeight")
        readNumber(fIn, sCurrentMark, &(fiFontInfo.BottomFrameHeight)); 
      if (sCurrentMark == "BottomFrameDepth")
        readNumber(fIn, sCurrentMark, &(fiFontInfo.BottomFrameDepth)); 
      if (sCurrentMark == "BottomNotationFrameHeight")
        readNumber(fIn, sCurrentMark, &(fiFontInfo.BottomNotationFrameHeight)); 
      if (sCurrentMark == "BottomNotationFrameDepth")
        readNumber(fIn, sCurrentMark, &(fiFontInfo.BottomNotationFrameDepth)); 
      if (sCurrentMark == "EpsScalingFactor")
        readNumber(fIn, sCurrentMark, &(fiFontInfo.ScaleFactor)); 

      if (sCurrentMark == "EpsBoardSize")
        readPSDimension(fIn, sCurrentMark, &(fiFontInfo.BoardSize)); 
      if (sCurrentMark == "EpsDefaultLineWidth")
        readPSDimension(fIn, sCurrentMark, &(fiFontInfo.LineWidth)); 
      if (sCurrentMark == "EpsLeftMargin")
        readPSDimension(fIn, sCurrentMark, &(fiFontInfo.LeftMargin)); 
      if (sCurrentMark == "EpsRightMargin")
        readPSDimension(fIn, sCurrentMark, &(fiFontInfo.RightMargin)); 
      if (sCurrentMark == "EpsTopMargin")
        readPSDimension(fIn, sCurrentMark, &(fiFontInfo.TopMargin)); 
      if (sCurrentMark == "EpsBottomMargin")
        readPSDimension(fIn, sCurrentMark, &(fiFontInfo.BottomMargin)); 
    }

    // Get next mark
    markType = getNextMark(fIn, sCurrentMark);
  }
  
  // Compute bounding box, translation and scaling factor
  // Was a board size specified?
  if (fiFontInfo.BoardSize > 0.0)
  {
    // Compute scaling factor
    fiFontInfo.ScaleFactor = fiFontInfo.BoardSize / (8 * fiFontInfo.SquareSize);
  } 

  // Compute bounding box
  if (bNotation == true)
  {
    fiFontInfo.BoundingBoxSizeX = fiFontInfo.LeftNotationFrameWidth * 
                                  fiFontInfo.ScaleFactor;
    fiFontInfo.BoundingBoxSizeY = (fiFontInfo.BottomNotationFrameHeight +
                                   fiFontInfo.BottomNotationFrameDepth) * 
                                   fiFontInfo.ScaleFactor;
  }
  else
  {
    fiFontInfo.BoundingBoxSizeX = fiFontInfo.LeftFrameWidth * 
                                  fiFontInfo.ScaleFactor;
    fiFontInfo.BoundingBoxSizeY = (fiFontInfo.BottomFrameHeight + 
                                   fiFontInfo.BottomFrameDepth) * 
                                   fiFontInfo.ScaleFactor;
  }
  fiFontInfo.BoundingBoxSizeX += fiFontInfo.LeftMargin +
                                 (8*fiFontInfo.SquareSize +
                                  fiFontInfo.RightFrameWidth) * fiFontInfo.ScaleFactor +
                                 fiFontInfo.RightMargin;
  fiFontInfo.BoundingBoxSizeY += fiFontInfo.BottomMargin +
                                 (8*fiFontInfo.SquareSize +
                                  fiFontInfo.TopFrameHeight +
                                  fiFontInfo.TopFrameDepth) * 
                                 fiFontInfo.ScaleFactor +
                                 fiFontInfo.TopMargin;

  // Compute translation
  if (bNotation == true)
  {
    fiFontInfo.TranslateX = (fiFontInfo.LeftNotationFrameWidth -
                             fiFontInfo.LeftFrameWidth) *
                             fiFontInfo.ScaleFactor;
    fiFontInfo.TranslateY = (fiFontInfo.BottomNotationFrameHeight +
                             fiFontInfo.BottomNotationFrameDepth) * 
                             fiFontInfo.ScaleFactor;
  }
  else
  {
    fiFontInfo.TranslateX = 0.0;
    fiFontInfo.TranslateY = (fiFontInfo.BottomFrameHeight +
                             fiFontInfo.BottomFrameDepth) * 
                             fiFontInfo.ScaleFactor;
  }
  fiFontInfo.TranslateX += fiFontInfo.LeftMargin;
  fiFontInfo.TranslateY += fiFontInfo.BottomMargin +
                           8*fiFontInfo.SquareSize*fiFontInfo.ScaleFactor + 
                           (fiFontInfo.TopFrameDepth *
                            fiFontInfo.ScaleFactor);
}

/** Maps the section name \a sSymbol to its ID.
@param sSymbol The name of the symbol
@return ID of the symbol, 13 (= white square) for unknown symbols
*/
int mapNameToID(const string &sSymbol)
{
  int i = 0;

  // Loop through all symbol names
  while (i < ciFontSymbols)
  {
    // Symbol found?
    if (pcSymbolNames[i] == sSymbol)
      return i;  // Yes, so return its ID
    i++;
  }

  // Nothing found, so return a blank/white square (WS)...
  return 13;
}

/** Exports the needed piece symbols to ``fOut''.
@param fOut The output file
@param fFont The font definition file
*/
void exportPieces(std::ostream &fOut, std::ifstream &fFont)
{
  // Current mark
  string sCurrentMark;
  // Type of found mark
  int markType = getNextMark(fFont, sCurrentMark);

  while (!fFont.eof())
  {
    // Skip to next ``BEGIN'' mark
    while ((!fFont.eof()) &&
           (markType != mtBegin))
      markType = getNextMark(fFont, sCurrentMark);

    // Is it the EPS preamble?
    if (sCurrentMark == "EpsPreamble")
    {
      // Yes, so export it
      writeSection(fFont, fOut, sCurrentMark);
    }
    else
    {
      // Do we have to export the found symbol?
      if (pbSymbolExport[mapNameToID(sCurrentMark)] == true)
      {
        // Yes
        fOut << "/F2E" << sCurrentMark << " {" << endl;
        writeSection(fFont, fOut, sCurrentMark, true);
        fOut << "} def" << endl;
      }
    }
    markType = getNextMark(fFont, sCurrentMark);
  }

  // Export ``space'' and ``newline'' commands...

  // Square width
  fOut << endl << "/F2ESW {" << fiFontInfo.SquareSize;
  fOut << " 0 translate} def" << endl;
  // Jump from left frame to first square in a row
  fOut << "/F2EFTOS {";
  if (bNotation == true)
  {
    fOut << fiFontInfo.LeftNotationFrameWidth;
    fOut << " " << (fiFontInfo.SquareDepth - fiFontInfo.LeftNotationFrameDepth);
  }
  else
  {
    fOut << fiFontInfo.LeftFrameWidth;
    fOut << " " << (fiFontInfo.SquareDepth - fiFontInfo.LeftFrameDepth);
  }
  fOut << " translate} def" << endl;
  // Jump from last square in a row to the right frame
  fOut << "/F2ESTOF {" << fiFontInfo.SquareSize;
  fOut << " " << (fiFontInfo.RightFrameDepth - fiFontInfo.SquareDepth);
  fOut << " translate} def" << endl;
  // New line
  fOut << "/F2ENL {-";
  if (bNotation == true)
  {
    fOut << (fiFontInfo.SquareSize*8 + fiFontInfo.LeftNotationFrameWidth);
    fOut << " -" << (fiFontInfo.SquareSize + fiFontInfo.LeftNotationFrameDepth -
                     fiFontInfo.RightFrameDepth);
    fOut << " translate} def" << endl;
  }
  else
  {
    fOut << (fiFontInfo.SquareSize*8 + fiFontInfo.LeftFrameWidth);
    fOut << " -" << (fiFontInfo.SquareSize + fiFontInfo.LeftFrameDepth -
                     fiFontInfo.RightFrameDepth);
    fOut << " translate} def" << endl;
  }
  fOut << endl; 

}


/** Writes the current board diagram to the file \a fOut.
@param fOut The output file
*/
void writeDiagram(std::ostream &fOut)
{
  // Counters
  int row, col;

  fOut << fiFontInfo.LineWidth << " setlinewidth" << endl;
  fOut << fiFontInfo.TranslateX;
  fOut << " " << fiFontInfo.TranslateY << " translate" << endl;
  fOut << fiFontInfo.ScaleFactor;
  fOut << " " << fiFontInfo.ScaleFactor << " scale" << endl;

  // Top frame
  fOut << "F2ELFUC" << endl;
  // Jump to first top frame
  fOut << fiFontInfo.LeftFrameWidth << " 0 translate" << endl;
  for (row = 0; row < 8; row++)
    fOut << "F2ETF F2ESW ";
  fOut << "F2ERFUC" << endl;

  // Jump to first line with notation
  fOut << "-";
  if (bNotation == true)
  {
    fOut << (fiFontInfo.SquareSize*8 + fiFontInfo.LeftNotationFrameWidth);
    fOut << " -" << (fiFontInfo.SquareSize - fiFontInfo.LeftNotationFrameDepth +
                     fiFontInfo.TopFrameDepth);
  }
  else
  {
    fOut << (fiFontInfo.SquareSize*8 + fiFontInfo.LeftFrameWidth);
    fOut << " -" << (fiFontInfo.SquareSize - fiFontInfo.LeftFrameDepth +
                     fiFontInfo.TopFrameDepth);
  }
  fOut << " translate" << endl;

  // Chess board
  for (row = 0; row < 8; row++)
  {
    // Left frame
    if (bNotation == true)
    {
      if (bReverse == true)
        fOut << "F2ELFN" << char('A' + row);
      else
        fOut << "F2ELFN" << char('A' + (7-row));
    }
    else
      fOut << "F2ELF";
    fOut << " F2EFTOS ";
  
    // Board rank
    for (col = 0; col < 7; col++)
    {
      fOut << "F2E" << pcSymbolNames[piCurrentBoard[row*8+col]];
      fOut << " F2ESW ";
    }
    fOut << "F2E" << pcSymbolNames[piCurrentBoard[row*8+col]];
    fOut << " F2ESTOF ";

    // Right frame
    fOut << "F2ERF";
    if (row < 7)
      fOut << " F2ENL" << endl;
    else
    {
      // Jump to left lower corner
      fOut << endl << "-";
      fOut << (fiFontInfo.SquareSize*8 + fiFontInfo.LeftFrameWidth);
      fOut << " -" << (fiFontInfo.BottomFrameHeight +
                       fiFontInfo.RightFrameDepth);
      fOut << " translate" << endl;
    }
  }

  // Bottom frame
  fOut << "F2ELFLC" << endl;
  // Jump from lower left corner to first bottom frame
  fOut << fiFontInfo.LeftFrameWidth << " ";
  if (bNotation == true)
    fOut << (fiFontInfo.BottomFrameHeight - fiFontInfo.BottomNotationFrameHeight);
  else
    fOut << "0";
  fOut << " translate" << endl;

  for (row = 0; row < 8; row++)
  {
    if (bNotation == true)
    {
      if (bReverse == true)
        fOut << "F2EBFN" << char('A' + (7-row));
      else
        fOut << "F2EBFN" << char('A' + row);
    }
    else
      fOut << "F2EBF";
    if (row < 7)
      fOut << " F2ESW ";
    else
    {
      // Jump to lower right corner
      fOut << endl << fiFontInfo.SquareSize << " ";
      if (bNotation == true)
        fOut << (fiFontInfo.BottomNotationFrameHeight - fiFontInfo.BottomFrameHeight);
      else
        fOut << "0";
      fOut << " translate" << endl;
    }
  }
  fOut << "F2ERFLC" << endl << endl;

}

/** Writes the EPS header to ``fOut''.
@param fOut The output file
*/
void writeEpsHeader(std::ostream &fOut)
{
  // Get the current time for creation date
  time_t currentTime = time(0);

  fOut << "%!PS-Adobe-2.0 EPSF-2.0" << endl;
  fOut << "%%Title: ";
  if (bPrefixExport == true)
    fOut << sOutFile << endl; 
  else
    fOut << "none" << endl;
  fOut << "%%Creator: fen2eps v1.0" << endl;
  
  fOut << "%%CreationDate: " << ctime(&currentTime) << endl;
  
  fOut << "%%For: " << endl;
  fOut << "%%Orientation: Portrait" << endl;
  fOut << "%%BoundingBox: 0 0 ";
  fOut << fiFontInfo.BoundingBoxSizeX << " ";
  fOut << fiFontInfo.BoundingBoxSizeY << endl;
  fOut << "%%Pages: 0" << endl;

  fOut << "%%BeginSetup" << endl;
  fOut << "%%EndSetup" << endl;
  fOut << "%%BeginFen2epsFontInfo" << endl;
  fOut << "%%F2E Name: " << fiFontInfo.FontName << endl;
  fOut << "%%F2E Author: " << fiFontInfo.FontAuthor << endl;
  fOut << "%%F2E Version: " << fiFontInfo.FontVersion << endl;
  fOut << "%%F2E Date: " << fiFontInfo.FontDate << endl;
  fOut << "%%EndFen2epsFontInfo" << endl;
  /* Magnification is set to 1 */
  fOut << "%%Magnification: 1.0000" << endl;
  fOut << "%%EndComments" << endl << endl;

  fOut << "save" << endl;
}

/** Writes the EPS trailer to ``fOut''.
@param fOut The output file
*/
void writeEpsTrailer(std::ostream &fOut)
{

  fOut << "restore" << endl << endl;
}

/** Display the ``usage message''.
*/
void usage()
{
  cerr << endl << "fen2eps v1.1 - by Dirk Baechle (dl9obn@darc.de), 2010-06-22" << endl;
  cerr << "Available at http://fen2eps.sourceforge.net" << endl;
  cerr << endl << "Usage:" << endl;
  cerr << "fen2eps [options]" << endl;
  cerr << endl << "Available options:" << endl;
  cerr << "-f <file_name>      Reads the font definitions from the file <file_name>." << endl;
  cerr << "-n                  Exports the board diagrams without notation." << endl;
  cerr << "-p <prefix>         Does not write the EPS diagrams to `stdout'" << endl;
  cerr << "                    but creates a single file for each FEN string." << endl;
  cerr << "                    File names start with <prefix> followed by a unique number." << endl;
  cerr << "-r                  Displays the boards reverse." << endl;
  cerr << endl << "Examples:" << endl;
  cerr << "fen2eps -r < a.fen > a.eps" << endl;
  cerr << "fen2eps -n -p diag -f fed/alpha.fed < a.fen" << endl << endl;
}



/*----------------------------------------------------------------- Main */

/** The ``main'' routine...the place where it all starts.
@param argc Number of arguments
@param argv Array of the arguments
*/
int main(int argc, char **argv)
{
  // The current input line
  string inputLine;
  // Counter
  int i;

  // Parse command-line arguments
  for (i = 1; i < argc; i++)
  {
    if (strcmp(argv[i],"-p") == 0)
    {
      // Last argument?
      if (i == argc)
        break;
      i++;
      sPrefix = argv[i];
      bPrefixExport = true;
    }
    if (strcmp(argv[i],"-f") == 0)
    {
      // Last argument?
      if (i == argc)
        break;
      i++;
      sFontFile = argv[i];
    }
    if (strcmp(argv[i],"-n") == 0)
    {
      bNotation = false;
    }
    if (strcmp(argv[i],"-r") == 0)
    {
      bReverse = true;
    }
    if (strcmp(argv[i],"-h") == 0)
    {
      usage();
      return 0;
    }
    if (strcmp(argv[i],"-?") == 0)
    {
      usage();
      return 0;
    }
    if (strcmp(argv[i],"--help") == 0)
    {
      usage();
      return 0;
    }
  } 

  // Open font definition file
  // Try to open the file
  std::ifstream *fFont = new std::ifstream(sFontFile.c_str());
  if (!(*fFont))
  {
    cerr << "Error: Could not open font definition file " << sFontFile << "!" << endl;
    return(1);
  }

  // Check file header...
  if (!checkFileHeader((*fFont)))
  {
    cerr << "Error: Wrong file header in Postscript font definition file " << sFontFile << "!" << endl;
    return(1);
  }

  // Read font infos from *.fed file...
  readFontInfos((*fFont));
  fFont->close();
  // Delete ifstream, for now...
  delete fFont;

  // Initialize frame export...
  for (i = 26; i < 34; i++)
    pbSymbolExport[i] = true;
  // Do we export with notation?
  if (bNotation == true)
  {
    // Yes, so kick out the simple left and bottom frame...
    pbSymbolExport[27] = false;
    pbSymbolExport[29] = false;
    //...and include the frames with notation
    for (i = 34; i < 50; i++)
      pbSymbolExport[i] = true;
  }
  else
  {
    // No, so set frames with notation to ``false''...
    for (i = 34; i < 50; i++)
      pbSymbolExport[i] = false;
  }

  // The output file
  std::ofstream *fOut;

  // Read from cin until EOF encountered...
  getline(cin, inputLine);
  while (!cin.eof())
  {
    lineNumber++;

    // Skip empty lines...
    if (inputLine.size() != 0)
    {
      if (expandFENString(inputLine) == true)
      {
        if (bPrefixExport == false)
        {
          // Write EPS header
          writeEpsHeader(cout);

          // Export the pieces...
          fFont = new std::ifstream(sFontFile.c_str());
          // Skip ``FontInfo'' section
          skipSection((*fFont), "FontInfo"); 
          exportPieces(cout, (*fFont));
          fFont->close();
          // Delete ifstream
          delete fFont;

          // Write chess diagram
          writeDiagram(cout);
        
          // Write EPS trailer
          writeEpsTrailer(cout);
        }
        else
        {
          fileNumber++;
          sFileNumber = "";
          // Open new file
          ostringstream fN(sFileNumber);
          fN << fileNumber;
          sFileNumber = fN.str();
          sOutFile = sPrefix + sFileNumber + ".eps";
          // The output file
          fOut = new std::ofstream(sOutFile.c_str());
          
          if (!(*fOut))
          {
            cerr << "Error: Could not open output file " << sOutFile << "!" << endl;
            delete fOut;
            break; 
          }

          // Write EPS header
          writeEpsHeader((*fOut));

          // Export the pieces...
          fFont = new std::ifstream(sFontFile.c_str());
          // Skip ``FontInfo'' section
          skipSection((*fFont), "FontInfo"); 
          exportPieces((*fOut), (*fFont));
          fFont->close();
          // Delete ifstream
          delete fFont;

          // Write chess diagram
          writeDiagram((*fOut));
        
          // Write EPS trailer
          writeEpsTrailer((*fOut));
          
          // Close file
          (*fOut).close();

          // Delete ofstream
          delete fOut;
        }
      }
    }

    // Read next line
    getline(cin, inputLine);
  }

  return(0);
}

