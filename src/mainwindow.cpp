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
#include "stylesheetparser/mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
   : QMainWindow(parent)
{
   setCentralWidget(initGui());
   setGeometry(0, 0, 500, 800);
}

MainWindow::~MainWindow() {}

QWidget* MainWindow::initGui()
{
   QFrame* f = new QFrame(this);
   QGridLayout* layout = new QGridLayout;
   f->setLayout(layout);

   QString text =
//     "\n\r\t";
//   for (auto c : text) {
//     if (c.isSpace()) {
//       qWarning() <<(c=="\t" ? "tab " : c=="\n" ? "NewLine " : c == "\r" ? "Return " : "") <<"is a space";
//     } else {
//       qWarning() <<(c=="\t" ? "tab " : c=="\n" ? "NewLine " : c == "\r" ? "Return " : "") <<"not a space";
//     }
//   }
//      " color: red;\n background: green;\n";
//     "QTabWidget {\n color: red;\n background: green;\n }\n";
   "  QTabWidget:active {\n}";

   m_editor = new StylesheetEdit(this);
   m_editor->setText(text);
   m_editor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
   layout->addWidget(m_editor, 0, 0);

   return f;
}
