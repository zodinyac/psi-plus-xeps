/*
 * history_widget.cpp
 * XEP-0313: Message Archive Management implementation plugin
 * Copyright (C) 2006 Nikita Belov <zodiac.nv@gmail.com>
 *
 */
 
#include "history_widget.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPlainTextEdit>

HistoryWidget::HistoryWidget(const QVector<MessageType> &Messages, QWidget *Parent) : QWidget(Parent)
{
    QString History;
    for (auto Message: Messages) {
        History += "[" + Message.DateTime.toString(Qt::ISODate) + "] " + Message.From + ": " + Message.Body + "\n";
    }
    
    QPlainTextEdit *TextEdit = new QPlainTextEdit(History, Parent);
    
    QHBoxLayout *Layout = new QHBoxLayout(Parent);
    Layout->addWidget(TextEdit);
    setLayout(Layout);
    
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Server history");
    show();    
}
