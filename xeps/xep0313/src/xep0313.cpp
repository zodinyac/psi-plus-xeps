/*
 * xep0313.cpp
 * XEP-0313: Message Archive Management implementation plugin
 * Copyright (C) 2006 Nikita Belov <zodiac.nv@gmail.com>
 *
 */

#include "xep0313.h"
#include "history_widget.h"

#include "stanzasendinghost.h"
#include "accountinfoaccessinghost.h"
#include "optionaccessinghost.h"
#include "contactinfoaccessinghost.h"

#include <QtCore/QDateTime>
#include <QtGui/QIcon>
#include <QtGui/QPixmap>
#include <QtWidgets/QAction>
#include <QtWidgets/QMessageBox>
#include <QtXml/QDomElement>
#include <QtXml/QDomNode>
#include <QtXml/QDomNodeList>

QString XEP0313::name() const
{
	return "XEP-0313: Message Archive Management implementation";
}

QString XEP0313::shortName() const
{
	return "XEP-0313";
}

QString XEP0313::version() const
{
	return "0.1";
}

QWidget *XEP0313::options()
{
	return nullptr;
}

bool XEP0313::enable()
{
    PluginEnabled = true;
    
    if (AIHost) {
        int Account = 0;
        while (AIHost->getId(Account) != "-1") {
            if (AIHost->getStatus(Account) != "offline") {
                SendFeaturesRequest(Account);
            }
            Account++;
        }
    }
    
    if (OAHost) {
        QStringList ActionsList = OAHost->getGlobalOption("options.ui.contactlist.toolbars.m0.actions").toStringList();
        if (!ActionsList.contains("xep0313-plugin")) {
            ActionsList << "xep0313-plugin";
            OAHost->setGlobalOption("options.ui.contactlist.toolbars.m0.actions", QVariant(ActionsList));
        }
    }
    
	return true;
}

bool XEP0313::disable()
{
    PluginEnabled = false;
    ServerSupportMam.clear();
    
	return true;
}

void XEP0313::applyOptions()
{
}

void XEP0313::restoreOptions()
{
}

QPixmap XEP0313::icon() const
{
    return QPixmap(":/xep0313.png");
}

QString XEP0313::pluginInfo()
{
    return tr("Author: ") + "Nikita Belov\n" +
           tr("Email: ") + "zodiac.nv@gmail.com\n" +
           trUtf8("XEP-0313: Message Archive Management implementation (http://xmpp.org/extensions/xep-0313.html).\n"
                  "It is a common desire for users of XMPP to want to store their messages in a central archive on their server. This feature allows them to record conversations that take place on clients that do not support local history storage, to synchronise conversation history seamlessly between multiple clients, to read the history of a MUC room, or to view old items in a pubsub node.\n");
}

bool XEP0313::incomingStanza(int Account, const QDomElement &Stanza)
{
    Q_UNUSED(Account)
    
    if (Stanza.tagName() == "iq" && Stanza.attribute("type") == "result") {
        QDomNodeList Queries = Stanza.elementsByTagName("query");
        for (int i = 0; i < Queries.length(); i++) {
            QDomNode QueryNode = Queries.item(i);
            if (QueryNode.isElement()) {
                QDomElement Query = QueryNode.toElement();
                if (Query.attribute("xmlns") == "http://jabber.org/protocol/disco#info") {
                    QDomNodeList Features = Query.elementsByTagName("feature");
                    for (int j = 0; j < Features.length(); j++) {
                        QDomNode FeatureNode = Features.item(j);
                        if (FeatureNode.isElement()) {
                            QDomElement Feature = FeatureNode.toElement();
                            if (Feature.attribute("var") == "urn:xmpp:mam:0" || Feature.attribute("var") == "urn:xmpp:mam:1") {
                                ServerSupportMam[Account] = true;
                            }
                        }
                    }
                }
            }
        }
    }
    
    if (Stanza.tagName() == "message" && !Stanza.hasAttribute("type")) {      
        QDomElement Delay = Stanza.firstChildElement("result").firstChildElement("forwarded").firstChildElement("delay");
        QDomElement Message = Stanza.firstChildElement("result").firstChildElement("forwarded").firstChildElement("message");
        if (!Message.isNull()) {
            LastRequestMUC = (Message.attribute("type") == "groupchat");
            
            QString From = Message.attribute("from");
            if (LastRequestMUC) {
                From = GetResourceFromJid(From);
            } else {
                From = RemoveResourceFromJid(From);
                if (AIHost && CIHost) {
                    QString Name;
                    if (From == RemoveResourceFromJid(Stanza.attribute("to"))) {
                        Name = AIHost->getNick(Account);
                    } else {
                        Name = CIHost->name(Account, From);
                    }
                    
                    if (!Name.isEmpty()) {
                        From = Name;
                    }
                }
            }

            LastRequestKey = Message.attribute("to");
            if (!LastRequestMUC) {
                LastRequestKey = RemoveResourceFromJid(LastRequestKey);
                if (LastRequestKey == RemoveResourceFromJid(Stanza.attribute("to"))) {
                    LastRequestKey = RemoveResourceFromJid(Message.attribute("from"));
                }
            }
            
            MessageType MessageObject;
            MessageObject.DateTime = QDateTime::fromString(Delay.attribute("stamp"), Qt::ISODate);
            MessageObject.From = From;
            MessageObject.Body = Message.firstChildElement("body").text();
            Messages[Account][LastRequestKey].append(MessageObject);
        } else {
            QString Last = Stanza.firstChildElement("fin").firstChildElement("set").firstChildElement("last").text();
            if (!Last.isEmpty()) {
                SendMAMRequest(Account, LastRequestMUC, LastRequestKey, Last);
            }
            
            QString Count = Stanza.firstChildElement("fin").firstChildElement("set").firstChildElement("count").text();
            if (!Count.isEmpty() && Last.isEmpty()) {
                new HistoryWidget(Messages[Account][LastRequestKey]);
            }
        }
    }
    
    return false;
}

bool XEP0313::outgoingStanza(int Account, QDomElement &Stanza)
{
    if (Stanza.tagName() == "presence") {
        if (Stanza.attribute("type") == "unavailable") {
            ServerSupportMam.remove(Account);
        } else {
            SendFeaturesRequest(Account);
        }
    }
    
    return false;
}

void XEP0313::setStanzaSendingHost(StanzaSendingHost *Host)
{
    SSHost = Host;
}

void XEP0313::setAccountInfoAccessingHost(AccountInfoAccessingHost *Host)
{
    AIHost = Host;
}

void XEP0313::setContactInfoAccessingHost(ContactInfoAccessingHost *Host)
{
    CIHost = Host;
}

void XEP0313::setOptionAccessingHost(OptionAccessingHost *Host)
{
    OAHost = Host;
}

void XEP0313::optionChanged(const QString &Option)
{
    Q_UNUSED(Option)
}

bool XEP0313::processEvent(int Account, QDomElement &e)
{
    Q_UNUSED(Account)
    Q_UNUSED(e)
    
    return false;
}

bool XEP0313::processMessage(int Account, const QString &FromJid, const QString &Body, const QString &Subject)
{
    Q_UNUSED(Account)
    Q_UNUSED(FromJid)
    Q_UNUSED(Body)
    Q_UNUSED(Subject)
    
    return false;
}

bool XEP0313::processOutgoingMessage(int Account, const QString &FromJid, QString &Body, const QString &Type, QString &Subject)
{
    Q_UNUSED(Account)
    Q_UNUSED(FromJid)
    Q_UNUSED(Body)
    Q_UNUSED(Type)
    Q_UNUSED(Subject)
    
    return false;
}

void XEP0313::logout(int Account)
{
    ServerSupportMam.remove(Account);
}

QList<QVariantHash> XEP0313::getGCButtonParam()
{
	return QList<QVariantHash>();
}

QAction *XEP0313::getGCAction(QObject *Parent, int Account, const QString &Contact)
{
    return getActionForToolBar(Parent, Account, Contact, true);
}

QList<QVariantHash> XEP0313::getButtonParam()
{
	return QList<QVariantHash>();
}

QAction *XEP0313::getAction(QObject *Parent, int Account, const QString &Contact)
{
    return getActionForToolBar(Parent, Account, Contact, false);
}

QAction *XEP0313::getActionForToolBar(QObject *Parent, int Account, const QString &Contact, bool MUC)
{
    QAction *ServerHistory = new QAction(Parent);
    ServerHistory->setIcon(QPixmap(":/server_history.png"));
	ServerHistory->setShortcut(QKeySequence("Ctrl+Shift+H"));
	ServerHistory->setShortcutContext(Qt::WindowShortcut);
    ServerHistory->setToolTip(tr("Server Message History"));

    connect(ServerHistory, &QAction::triggered, [this, Parent, Account, Contact, MUC]() {
        if (ServerSupportMam.contains(Account)) {
            QMessageBox::StandardButton Result = QMessageBox::question(qobject_cast<QWidget *>(Parent),
                                                                       tr("Do you want to continue?"),
                                                                       tr("This operation can take a lot of time. Do you want to continue?"));
            if (Result == QMessageBox::Yes) {
                SendMAMRequest(Account, MUC, Contact);
            }
        } else {
            QMessageBox::critical(qobject_cast<QWidget *>(Parent),
                                  tr("Server history unavailable"),
                                  tr("This server doesn't support server history or you are not connected to server."));
        }
    });
    
    return ServerHistory;
}

void XEP0313::SendFeaturesRequest(int Account)
{
    if (!SSHost || !AIHost) {
        return;
    }
    
    QDomDocument Document;
    
    QDomElement Stanza = Document.createElement("iq");
    Stanza.setAttribute("type", "get");
    Stanza.setAttribute("id", SSHost->uniqueId(Account));
    Stanza.setAttribute("to", AIHost->getName(Account));
    
    QDomElement Query = Document.createElement("query");
    Query.setAttribute("xmlns", "http://jabber.org/protocol/disco#info");
    Stanza.appendChild(Query);    
    
    SSHost->sendStanza(Account, Stanza);
}

void XEP0313::SendMAMRequest(int Account, bool MUC, const QString &Jid, const QString &AfterMessageId)
{
    if (!SSHost) {
        return;
    }
    
    QString UniqueId = SSHost->uniqueId(Account);
    
    QDomDocument Document;
    
    QDomElement Stanza = Document.createElement("iq");
    Stanza.setAttribute("type", "set");
    Stanza.setAttribute("id", UniqueId);
    if (MUC) {
        Stanza.setAttribute("to", Jid);
    }
    
    QDomElement Query = Document.createElement("query");
    Query.setAttribute("xmlns", "urn:xmpp:mam:0");
    
    QDomElement X = Document.createElement("x");
    X.setAttribute("xmlns", "jabber:x:data");
    X.setAttribute("type", "submit");
    
    {
        QDomElement Field = Document.createElement("field");
        Field.setAttribute("var", "FORM_TYPE");
        Field.setAttribute("type", "hidden");
        
        QDomElement Value = Document.createElement("value");
        QDomText ValueText = Document.createTextNode("urn:xmpp:mam:0");
        
        Value.appendChild(ValueText);
        Field.appendChild(Value);
        X.appendChild(Field);
    }
    
    if (!MUC) {
        QDomElement Field = Document.createElement("field");
        Field.setAttribute("var", "with");
        
        QDomElement Value = Document.createElement("value");
        QDomText ValueText = Document.createTextNode(Jid);
        
        Value.appendChild(ValueText);
        Field.appendChild(Value);
        X.appendChild(Field);
    }
    
    Query.appendChild(X);
    
    if (!AfterMessageId.isEmpty()) {
        QDomElement Set = Document.createElement("set");
        Set.setAttribute("xmlns", "http://jabber.org/protocol/rsm");
        
        QDomElement After = Document.createElement("after");
        QDomText AfterText = Document.createTextNode(AfterMessageId);
        After.appendChild(AfterText);
        
        Set.appendChild(After);
        Query.appendChild(Set);
    }
    
    Stanza.appendChild(Query);
    
    SSHost->sendStanza(Account, Stanza);
}

QString XEP0313::GetResourceFromJid(const QString &Jid)
{
    return Jid.section('/', 1);
}

QString XEP0313::RemoveResourceFromJid(const QString &Jid)
{
    return Jid.section('/', 0, 0);
}

void XEP0313::WriteDebugMessage(const QString &Message)
{
    QFile File("D:/Devel/git/psi-plus-snapshots/src/plugins/generic/xep0313/debug.txt");
    File.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    
    QTextStream OutStream(&File);
    OutStream << QDateTime::currentDateTime().toString(Qt::ISODate) << ": " << Message << "\n";
    
    File.close();
}
    