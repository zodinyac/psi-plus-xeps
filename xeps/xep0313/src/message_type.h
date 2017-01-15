/*
 * message_type.h
 * XEP-0313: Message Archive Management implementation plugin
 * Copyright (C) 2006 Nikita Belov <zodiac.nv@gmail.com>
 *
 */
 
#ifndef MESSAGE_TYPE_H
#define MESSAGE_TYPE_H

#include <QtCore/QDateTime>
#include <QtCore/QString>

struct MessageType
{
    QDateTime DateTime;
    QString From;
    QString Body;
};
    
#endif
