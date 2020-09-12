/*
   Copyright 2020 Simon Meaden

   Permission is hereby granted, free of charge, to any person obtaining a copy of this
   software and associated documentation files (the "Software"), to deal in the Software
   without restriction, including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software, and to permit
                                                                         persons to whom the Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all copies or
   substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
   INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
    PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
   OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
      SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef STYLESHEETPARSER_H
#define STYLESHEETPARSER_H

#include <QObject>
#include <QtDebug>

#include "StylesheetParser_global.h"
#include "parserstate.h"
#include "node.h"
#include "datastore.h"

namespace StylesheetParser {

class STYLESHEETPARSER_EXPORT Parser : public QObject
{
public:
  Parser(QObject* parent = nullptr);
  ~Parser();

  ParserState* parse(const QString& text);

  NodeList* nodes();

private:
  NodeList* m_nodes;
  DataStore* m_datastore;
  int m_braceCount;
  bool m_bracesMatched;

  // Skips blank characters (inc \n\t etc.) and returns the first non-blank character.
  void skipBlanks(const QString& text, int& pos);

  QString findNext(const QString& text, int& pos, ParserState* state);
  void deleteNodes();
  void setNodeLinks(Node* first, Node* second);
  void checkBraceCount(const QString& text, ParserState* state);
};

} // end of StylesheetParser

#endif // STYLESHEETPARSER_H
