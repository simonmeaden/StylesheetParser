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

  QString text;
  //      " color: red;\n background: green;\n";
//  text = "QTabWidget::branch {\n}\n\n";
  //  text = "QTabWidget::branch {\n}"
//    text = "QTabWidget:active { border: 1px solid rd\n }";
    text = "QTabWidget::branch {\n color: black;\n border: 1px solid rd\n }";
//  text = "color: black; border: 1px solid rd\n";


  m_editor = new StylesheetEdit(this);
  m_editor->setPlainText(text);
  m_editor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->addWidget(m_editor, 0, 0);

  return f;
}
