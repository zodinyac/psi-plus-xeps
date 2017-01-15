/*
 * xep0313.h
 * XEP-0313: Message Archive Management implementation plugin
 * Copyright (C) 2006 Nikita Belov <zodiac.nv@gmail.com>
 *
 */
 
#ifndef XEP0313_H
#define XEP0313_H

#include "message_type.h"

#include "psiplugin.h"
#include "plugininfoprovider.h"
#include "stanzafilter.h"
#include "stanzasender.h"
#include "accountinfoaccessor.h"
#include "eventfilter.h"
#include "gctoolbariconaccessor.h"
#include "toolbariconaccessor.h"
#include "optionaccessor.h"

#include <QtCore/QMap>
#include <QtCore/QVector>

class XEP0313 : public QObject,
                public PsiPlugin,
                public PluginInfoProvider,
                public StanzaFilter,
                public StanzaSender,
                public AccountInfoAccessor,
                public EventFilter,
                public GCToolbarIconAccessor,
                public ToolbarIconAccessor,
                public OptionAccessor
{
	Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.psi-plus.XEP0313")
	Q_INTERFACES(PsiPlugin PluginInfoProvider StanzaFilter StanzaSender AccountInfoAccessor EventFilter GCToolbarIconAccessor ToolbarIconAccessor OptionAccessor)

public:
	QString name() const;
	QString shortName() const;
	QString version() const;
	QWidget *options();
	bool enable();
	bool disable();
    void applyOptions();
    void restoreOptions();
    QPixmap icon() const;
    
    QString pluginInfo();
    
    bool incomingStanza(int Account, const QDomElement &Stanza);
    bool outgoingStanza(int Account, QDomElement &Stanza);
    
    void setStanzaSendingHost(StanzaSendingHost *Host);
    void setAccountInfoAccessingHost(AccountInfoAccessingHost *Host);
    
    void setOptionAccessingHost(OptionAccessingHost *Host);
	void optionChanged(const QString &Option);
    
    bool processEvent(int Account, QDomElement &e);
    bool processMessage(int Account, const QString &FromJid, const QString &Body, const QString &Subject);
    bool processOutgoingMessage(int Account, const QString &FromJid, QString &Body, const QString &Type, QString &Subject);
    void logout(int Account);
    
    QList<QVariantHash> getGCButtonParam();
	QAction *getGCAction(QObject *Parent, int Account, const QString &Contact);
    
    QList<QVariantHash> getButtonParam();
	QAction *getAction(QObject *Parent, int Account, const QString &Contact);
    
private:
    QAction *getActionForToolBar(QObject *Parent, int Account, const QString &Contact, bool MUC);
    
    void SendFeaturesRequest(int Account);
    void SendMAMRequest(int Account, bool MUC, const QString &Jid, const QString &AfterMessageId = QString());
    
    bool PluginEnabled = false;
    QMap<int, bool> ServerSupportMam;
    
    QMap<int, QMap<QString, QVector<MessageType>>> Messages;
    
    StanzaSendingHost *SSHost = nullptr;
    AccountInfoAccessingHost *AIHost = nullptr;
    OptionAccessingHost *OAHost = nullptr;
    
    
    
    
    void WriteDebugMessage(const QString &Message);
};

#endif
