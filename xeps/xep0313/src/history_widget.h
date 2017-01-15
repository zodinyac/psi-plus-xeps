/*
 * history_widget.h
 * XEP-0313: Message Archive Management implementation plugin
 * Copyright (C) 2006 Nikita Belov <zodiac.nv@gmail.com>
 *
 */
 
#ifndef HISTORY_WIDGET_H
#define HISTORY_WIDGET_H

#include "message_type.h"

#include <QtCore/QVector>
#include <QtWidgets/QWidget>

class HistoryWidget : public QWidget
{
    Q_OBJECT
    
public:
    HistoryWidget(const QVector<MessageType> &Messages, QWidget *Parent = 0);
};

#endif
