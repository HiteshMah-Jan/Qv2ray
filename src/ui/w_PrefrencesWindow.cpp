#include "QvUtils.h"
#include "QvCoreInteractions.h"
#include "w_PrefrencesWindow.h"
#include <QFileDialog>
#include <QColorDialog>

#include <iostream>

#define LOADINGCHECK if(!finishedLoading) return;
#define NEEDRESTART if(finishedLoading) IsConnectionPropertyChanged = true;

PrefrencesWindow::PrefrencesWindow(QWidget *parent) : QDialog(parent),
    CurrentConfig(),
    ui(new Ui::PrefrencesWindow)
{
    ui->setupUi(this);
    // We add locales
    ui->languageComboBox->clear();
    QDirIterator it(":/translations");

    while (it.hasNext()) {
        ui->languageComboBox->addItem(it.next().split("/").last().split(".").first());
    }

    for (auto item : NetSpeedPluginMessages.values()) {
        ui->nsBarContentCombo->addItem(item);
    }

    //
    ui->qvVersion->setText(QV2RAY_VERSION_STRING);
    CurrentConfig = GetGlobalConfig();
    //
    ui->languageComboBox->setCurrentText(QSTRING(CurrentConfig.language));
    ui->logLevelComboBox->setCurrentIndex(CurrentConfig.logLevel);
    ui->tProxyCheckBox->setChecked(CurrentConfig.tProxySupport);
    //
    //
    ui->listenIPTxt->setText(QSTRING(CurrentConfig.inBoundSettings.listenip));
    //
    bool have_http = CurrentConfig.inBoundSettings.http_port != 0;
    ui->httpCB->setChecked(have_http);
    ui->httpPortLE->setValue(CurrentConfig.inBoundSettings.http_port);
    ui->httpAuthCB->setChecked(CurrentConfig.inBoundSettings.http_useAuth);
    //
    ui->httpAuthCB->setEnabled(have_http);
    ui->httpAuthCB->setChecked(CurrentConfig.inBoundSettings.http_useAuth);
    ui->httpAuthUsernameTxt->setEnabled(have_http && CurrentConfig.inBoundSettings.http_useAuth);
    ui->httpAuthPasswordTxt->setEnabled(have_http && CurrentConfig.inBoundSettings.http_useAuth);
    ui->httpAuthUsernameTxt->setText(QSTRING(CurrentConfig.inBoundSettings.httpAccount.user));
    ui->httpAuthPasswordTxt->setText(QSTRING(CurrentConfig.inBoundSettings.httpAccount.pass));
    //
    //
    bool have_socks = CurrentConfig.inBoundSettings.socks_port != 0;
    ui->socksCB->setChecked(have_socks);
    ui->socksPortLE->setValue(CurrentConfig.inBoundSettings.socks_port);
    ui->socksAuthCB->setChecked(CurrentConfig.inBoundSettings.socks_useAuth);
    //
    ui->socksAuthCB->setEnabled(have_socks);
    ui->socksAuthCB->setChecked(CurrentConfig.inBoundSettings.socks_useAuth);
    ui->socksAuthUsernameTxt->setEnabled(have_socks && CurrentConfig.inBoundSettings.socks_useAuth);
    ui->socksAuthPasswordTxt->setEnabled(have_socks && CurrentConfig.inBoundSettings.socks_useAuth);
    ui->socksAuthUsernameTxt->setText(QSTRING(CurrentConfig.inBoundSettings.socksAccount.user));
    ui->socksAuthPasswordTxt->setText(QSTRING(CurrentConfig.inBoundSettings.socksAccount.pass));
    // Socks UDP Options
    ui->socksUDPCB->setChecked(CurrentConfig.inBoundSettings.socksUDP);
    ui->socksUDPIP->setEnabled(CurrentConfig.inBoundSettings.socksUDP);
    ui->socksUDPIP->setText(QSTRING(CurrentConfig.inBoundSettings.socksLocalIP));
    //
    //
    ui->vCoreAssetsPathTxt->setText(QSTRING(CurrentConfig.v2AssetsPath));
    ui->statsCheckbox->setChecked(CurrentConfig.enableStats);
    ui->statsPortBox->setValue(CurrentConfig.statsPort);
    //
    //
#if false
    ui->muxEnabledCB->setChecked(CurrentConfig.mux.enabled);
    ui->muxConcurrencyTxt->setValue(CurrentConfig.mux.concurrency);
#endif
    //
    //
    ui->bypassCNCb->setChecked(CurrentConfig.bypassCN);
    ui->proxyDefaultCb->setChecked(CurrentConfig.enableProxy);
    //
    ui->localDNSCb->setChecked(CurrentConfig.withLocalDNS);
    //
    ui->DNSListTxt->clear();

    foreach (auto dnsStr, CurrentConfig.dnsList) {
        auto str = QString::fromStdString(dnsStr).trimmed();

        if (!str.isEmpty()) {
            ui->DNSListTxt->appendPlainText(str);
        }
    }

    foreach (auto connection, CurrentConfig.configs) {
        ui->autoStartCombo->addItem(QSTRING(connection));
    }

    ui->autoStartCombo->setCurrentText(QSTRING(CurrentConfig.autoStartConfig));
    ui->cancelIgnoreVersionBtn->setEnabled(CurrentConfig.ignoredVersion != "");
    ui->ignoredNextVersion->setText(QSTRING(CurrentConfig.ignoredVersion));
    //
    // TODO : Show nsBarPageList content.
    //

    for (size_t i = 0; i < CurrentConfig.speedBarConfig.Pages.size(); i++) {
        ui->nsBarPagesList->addItem(tr("Page") + QString::number(i));
    }

    if (CurrentConfig.speedBarConfig.Pages.size() > 0) {
        ui->nsBarPagesList->setCurrentRow(0);
        on_nsBarPagesList_currentRowChanged(0);
    }

    CurrentBarPageId = 0;
    finishedLoading = true;
}

PrefrencesWindow::~PrefrencesWindow()
{
    delete ui;
}

void PrefrencesWindow::on_buttonBox_accepted()
{
    int sp = ui->socksPortLE->text().toInt();
    int hp = ui->httpPortLE->text().toInt() ;

    if (!(sp == 0 || hp == 0) && sp == hp) {
        QvMessageBox(this, tr("Prefrences"), tr("Port numbers cannot be the same"));
        return;
    }

    SetGlobalConfig(CurrentConfig);
    emit s_reload_config(IsConnectionPropertyChanged);
}

void PrefrencesWindow::on_httpCB_stateChanged(int checked)
{
    NEEDRESTART
    ui->httpPortLE->setEnabled(checked == Qt::Checked);
    ui->httpAuthCB->setEnabled(checked == Qt::Checked);
    ui->httpAuthUsernameTxt->setEnabled(checked == Qt::Checked && ui->httpAuthCB->isChecked());
    ui->httpAuthPasswordTxt->setEnabled(checked == Qt::Checked && ui->httpAuthCB->isChecked());
    CurrentConfig.inBoundSettings.http_port = checked == Qt::Checked ? CurrentConfig.inBoundSettings.http_port : 0;

    if (checked != Qt::Checked) {
        ui->httpPortLE->setValue(0);
    }
}

void PrefrencesWindow::on_socksCB_stateChanged(int checked)
{
    NEEDRESTART
    ui->socksPortLE->setEnabled(checked == Qt::Checked);
    ui->socksAuthCB->setEnabled(checked == Qt::Checked);
    ui->socksAuthUsernameTxt->setEnabled(checked == Qt::Checked && ui->socksAuthCB->isChecked());
    ui->socksAuthPasswordTxt->setEnabled(checked == Qt::Checked && ui->socksAuthCB->isChecked());
    CurrentConfig.inBoundSettings.socks_port = checked == Qt::Checked ? CurrentConfig.inBoundSettings.socks_port : 0;

    if (checked != Qt::Checked) {
        ui->socksPortLE->setValue(0);
    }
}

void PrefrencesWindow::on_httpAuthCB_stateChanged(int checked)
{
    NEEDRESTART
    ui->httpAuthUsernameTxt->setEnabled(checked == Qt::Checked);
    ui->httpAuthPasswordTxt->setEnabled(checked == Qt::Checked);
    CurrentConfig.inBoundSettings.http_useAuth = checked == Qt::Checked;
}

void PrefrencesWindow::on_socksAuthCB_stateChanged(int checked)
{
    NEEDRESTART
    ui->socksAuthUsernameTxt->setEnabled(checked == Qt::Checked);
    ui->socksAuthPasswordTxt->setEnabled(checked == Qt::Checked);
    CurrentConfig.inBoundSettings.socks_useAuth = checked == Qt::Checked;
}

void PrefrencesWindow::on_languageComboBox_currentTextChanged(const QString &arg1)
{
    CurrentConfig.language = arg1.toStdString();
    //
    // A strange bug prevents us to change the UI language online
    //    https://github.com/lhy0403/Qv2ray/issues/34
    //
    //if (QApplication::installTranslator(getTranslator(&arg1))) {
    //    LOG(MODULE_UI, "Loaded translations " + arg1.toStdString())
    //    ui->retranslateUi(this);
    //} else {
    //    QvMessageBox(this, tr("#Prefrences"), tr("#SwitchTranslationError"));
    //}
}

void PrefrencesWindow::on_logLevelComboBox_currentIndexChanged(int index)
{
    NEEDRESTART
    CurrentConfig.logLevel = index;
}

void PrefrencesWindow::on_vCoreAssetsPathTxt_textEdited(const QString &arg1)
{
    NEEDRESTART
    CurrentConfig.v2AssetsPath = arg1.toStdString();
}

void PrefrencesWindow::on_listenIPTxt_textEdited(const QString &arg1)
{
    NEEDRESTART
    CurrentConfig.inBoundSettings.listenip = arg1.toStdString();
}

void PrefrencesWindow::on_httpAuthUsernameTxt_textEdited(const QString &arg1)
{
    NEEDRESTART
    CurrentConfig.inBoundSettings.httpAccount.user = arg1.toStdString();
}

void PrefrencesWindow::on_httpAuthPasswordTxt_textEdited(const QString &arg1)
{
    NEEDRESTART
    CurrentConfig.inBoundSettings.httpAccount.pass = arg1.toStdString();
}

void PrefrencesWindow::on_socksAuthUsernameTxt_textEdited(const QString &arg1)
{
    NEEDRESTART
    CurrentConfig.inBoundSettings.socksAccount.user = arg1.toStdString();
}

void PrefrencesWindow::on_socksAuthPasswordTxt_textEdited(const QString &arg1)
{
    NEEDRESTART
    CurrentConfig.inBoundSettings.socksAccount.pass = arg1.toStdString();
}

void PrefrencesWindow::on_proxyDefaultCb_stateChanged(int arg1)
{
    NEEDRESTART
    CurrentConfig.enableProxy = arg1 == Qt::Checked;
}

void PrefrencesWindow::on_localDNSCb_stateChanged(int arg1)
{
    NEEDRESTART
    CurrentConfig.withLocalDNS = arg1 == Qt::Checked;
}

void PrefrencesWindow::on_selectVAssetBtn_clicked()
{
    NEEDRESTART
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open v2ray assets folder"), QDir::currentPath());
    ui->vCoreAssetsPathTxt->setText(dir);
    on_vCoreAssetsPathTxt_textEdited(dir);
}

void PrefrencesWindow::on_DNSListTxt_textChanged()
{
    if (finishedLoading) {
        try {
            QStringList hosts = ui->DNSListTxt->toPlainText().replace("\r", "").split("\n");
            CurrentConfig.dnsList.clear();

            foreach (auto host, hosts) {
                if (host != "" && host != "\r") {
                    // Not empty, so we save.
                    CurrentConfig.dnsList.push_back(host.toStdString());
                    NEEDRESTART
                }
            }

            BLACK(DNSListTxt)
        } catch (...) {
            RED(DNSListTxt)
        }
    }
}

void PrefrencesWindow::on_autoStartCombo_currentTextChanged(const QString &arg1)
{
    CurrentConfig.autoStartConfig = arg1.toStdString();
}

void PrefrencesWindow::on_aboutQt_clicked()
{
    QApplication::aboutQt();
}

void PrefrencesWindow::on_cancelIgnoreVersionBtn_clicked()
{
    CurrentConfig.ignoredVersion.clear();
    ui->cancelIgnoreVersionBtn->setEnabled(false);
}

void PrefrencesWindow::on_tProxyCheckBox_stateChanged(int arg1)
{
#ifdef __linux

    if (finishedLoading) {
        //LOG(MODULE_UI, "Running getcap....")
        //QProcess::execute("getcap " + QV2RAY_V2RAY_CORE_PATH);

        // Set UID and GID for linux
        // Steps:
        // --> 1. Copy v2ray core files to the #CONFIG_DIR#/vcore/ dir.
        // --> 2. Change GlobalConfig.v2CorePath.
        // --> 3. Call `pkexec setcap CAP_NET_ADMIN,CAP_NET_RAW,CAP_NET_BIND_SERVICE=eip` on the v2ray core.
        if (arg1 == Qt::Checked) {
            // We enable it!
            if (QvMessageBoxAsk(this, tr("Enable tProxy Support"), tr("This will append capabilities to the v2ray executable.")  + "\r\n"
                                + tr("If anything goes wrong after enabling this, please refer to issue #57 or the link below:") + "\r\n" +
                                " https://github.com/lhy0403/Qv2ray/blob/master/docs/FAQ.md ") != QMessageBox::Yes) {
                ui->tProxyCheckBox->setChecked(false);
                LOG(MODULE_UI, "Canceled enabling tProxy feature.")
            }

            int ret = QProcess::execute("pkexec setcap CAP_NET_ADMIN,CAP_NET_RAW,CAP_NET_BIND_SERVICE=eip " + QV2RAY_V2RAY_CORE_PATH);

            if (ret != 0) {
                LOG(MODULE_UI, "WARN: setcap exits with code: " + to_string(ret))
                QvMessageBox(this, tr("Prefrences"), tr("Failed to setcap onto v2ray executable. You may need to run `setcap` manually."));
            }

            CurrentConfig.tProxySupport = true;
            NEEDRESTART
        } else {
            int ret = QProcess::execute("pkexec setcap -r " + QV2RAY_V2RAY_CORE_PATH);

            if (ret != 0) {
                LOG(MODULE_UI, "WARN: setcap exits with code: " + to_string(ret))
                QvMessageBox(this, tr("Prefrences"), tr("Failed to setcap onto v2ray executable. You may need to run `setcap` manually."));
            }

            CurrentConfig.tProxySupport = false;
            NEEDRESTART
        }
    }

#else
    Q_UNUSED(arg1)
    ui->tProxyCheckBox->setChecked(false);
    // No such uid gid thing on Windows and macOS
    QvMessageBox(this, tr("Prefrences"), tr("tProxy is not supported on macOS and Windows"));
#endif
}
void PrefrencesWindow::on_bypassCNCb_stateChanged(int arg1)
{
    NEEDRESTART
    CurrentConfig.bypassCN = arg1 == Qt::Checked;
}

void PrefrencesWindow::on_statsCheckbox_stateChanged(int arg1)
{
    NEEDRESTART
    CurrentConfig.enableStats = arg1 == Qt::Checked;
}

void PrefrencesWindow::on_statsPortBox_valueChanged(int arg1)
{
    NEEDRESTART
    CurrentConfig.statsPort = arg1;
}

void PrefrencesWindow::on_socksPortLE_valueChanged(int arg1)
{
    NEEDRESTART
    CurrentConfig.inBoundSettings.socks_port = arg1;
}

void PrefrencesWindow::on_httpPortLE_valueChanged(int arg1)
{
    NEEDRESTART
    CurrentConfig.inBoundSettings.http_port = arg1;
}

void PrefrencesWindow::on_socksUDPCB_stateChanged(int arg1)
{
    NEEDRESTART
    CurrentConfig.inBoundSettings.socksUDP = arg1 == Qt::Checked;
    ui->socksUDPIP->setEnabled(arg1 == Qt::Checked);
}

void PrefrencesWindow::on_socksUDPIP_textEdited(const QString &arg1)
{
    NEEDRESTART
    CurrentConfig.inBoundSettings.socksLocalIP = arg1.toStdString();
}

// ------------------- NET SPEED PLUGIN OPERATIONS -----------------------------------------------------------------

#define CurrentBarPage CurrentConfig.speedBarConfig.Pages[this->CurrentBarPageId]
#define CurrentBarLine CurrentBarPage.Lines[this->CurrentBarLineId]
#define SET_LINE_LIST_TEXT ui->nsBarLinesList->currentItem()->setText(GetBarLineDescription(CurrentBarLine));

void PrefrencesWindow::on_nsBarPageAddBTN_clicked()
{
    QvBarPage page;
    CurrentConfig.speedBarConfig.Pages.push_back(page);
    CurrentBarPageId = CurrentConfig.speedBarConfig.Pages.size() - 1 ;
    // Add default line.
    QvBarLine line;
    CurrentBarPage.Lines.push_back(line);
    CurrentBarLineId = 0;
    ui->nsBarPagesList->addItem(QString::number(CurrentBarPageId));
    ShowLineParameters(CurrentBarLine);
    LOG(MODULE_UI, "Adding new page Id: " + to_string(CurrentBarPageId))
}

void PrefrencesWindow::on_nsBarPageDelBTN_clicked()
{
    RemoveItem(CurrentConfig.speedBarConfig.Pages, static_cast<size_t>(ui->nsBarPagesList->currentRow()));
    ui->nsBarPagesList->takeItem(ui->nsBarPagesList->currentRow());
}

void PrefrencesWindow::on_nsBarPageYOffset_valueChanged(int arg1)
{
    LOADINGCHECK
    CurrentBarPage.OffsetYpx = arg1;
}

void PrefrencesWindow::on_nsBarLineAddBTN_clicked()
{
    // WARNING Is it really just this simple?
    QvBarLine line;
    CurrentBarPage.Lines.push_back(line);
    CurrentBarLineId = CurrentBarPage.Lines.size() - 1 ;
    ui->nsBarLinesList->addItem(QString::number(CurrentBarLineId));
    ShowLineParameters(CurrentBarLine);
    LOG(MODULE_UI, "Adding new line Id: " + to_string(CurrentBarLineId))
    // TODO Some UI Works such as enabling ui.
}

void PrefrencesWindow::on_nsBarLineDelBTN_clicked()
{
    RemoveItem(CurrentBarPage.Lines, static_cast<size_t>(ui->nsBarLinesList->currentRow()));
    ui->nsBarLinesList->takeItem(ui->nsBarLinesList->currentRow());
    CurrentBarLineId = 0;
    // TODO Disabling some UI;
}

void PrefrencesWindow::on_nsBarPagesList_currentRowChanged(int currentRow)
{
    if (currentRow < 0) return;

    // Change page.
    // We reload the lines
    // Set all parameters item to the property of the first line.
    CurrentBarPageId = static_cast<size_t>(currentRow);
    CurrentBarLineId = 0;
    ui->nsBarPageYOffset->setValue(CurrentBarPage.OffsetYpx);
    ui->nsBarLinesList->clear();

    if (!CurrentBarPage.Lines.empty()) {
        for (auto line : CurrentBarPage.Lines) {
            auto description = GetBarLineDescription(line);
            ui->nsBarLinesList->addItem(description);
        }

        ui->nsBarLinesList->setCurrentRow(0);
        ShowLineParameters(CurrentBarLine);
    }
}

void PrefrencesWindow::on_nsBarLinesList_currentRowChanged(int currentRow)
{
    if (currentRow < 0) return;

    CurrentBarLineId = static_cast<size_t>(currentRow);
    ShowLineParameters(CurrentBarLine);
}

void PrefrencesWindow::on_fontComboBox_currentFontChanged(const QFont &f)
{
    LOADINGCHECK
    CurrentBarLine.Family = f.family().toStdString();
    SET_LINE_LIST_TEXT
}

void PrefrencesWindow::on_nsBarFontBoldCB_stateChanged(int arg1)
{
    LOADINGCHECK
    CurrentBarLine.Bold = arg1 == Qt::Checked;
    SET_LINE_LIST_TEXT
}

void PrefrencesWindow::on_nsBarFontItalicCB_stateChanged(int arg1)
{
    LOADINGCHECK
    CurrentBarLine.Italic = arg1 == Qt::Checked;
    SET_LINE_LIST_TEXT
}

void PrefrencesWindow::on_nsBarFontASB_valueChanged(int arg1)
{
    LOADINGCHECK
    CurrentBarLine.ColorA = arg1;
    ShowLineParameters(CurrentBarLine);
    SET_LINE_LIST_TEXT
}

void PrefrencesWindow::on_nsBarFontRSB_valueChanged(int arg1)
{
    LOADINGCHECK
    CurrentBarLine.ColorR = arg1;
    ShowLineParameters(CurrentBarLine);
    SET_LINE_LIST_TEXT
}

void PrefrencesWindow::on_nsBarFontGSB_valueChanged(int arg1)
{
    LOADINGCHECK
    CurrentBarLine.ColorG = arg1;
    ShowLineParameters(CurrentBarLine);
    SET_LINE_LIST_TEXT
}

void PrefrencesWindow::on_nsBarFontBSB_valueChanged(int arg1)
{
    LOADINGCHECK
    CurrentBarLine.ColorB = arg1;
    ShowLineParameters(CurrentBarLine);
    SET_LINE_LIST_TEXT
}

void PrefrencesWindow::on_nsBarFontSizeSB_valueChanged(double arg1)
{
    LOADINGCHECK
    CurrentBarLine.Size = arg1;
    SET_LINE_LIST_TEXT
}

QString PrefrencesWindow::GetBarLineDescription(QvBarLine line)
{
    QString result = "Empty";
    result = NetSpeedPluginMessages[line.ContentType];

    // BUG Content type is null, then set empty;
    if (line.ContentType == 0) {
        result +=  "(" + QSTRING(line.Message) + ")";
    }

    result = result.append(line.Bold ?  ", " + tr("Bold") : "");
    result = result.append(line.Italic ? ", " + tr("Italic") : "");
    // TODO : Set more descriptions
    return result;
}

void PrefrencesWindow::ShowLineParameters(QvBarLine &line)
{
    finishedLoading = false;

    if (!line.Family.empty()) {
        ui->fontComboBox->setCurrentFont(QFont(QSTRING(line.Family)));
    }

    // Colors
    ui->nsBarFontASB->setValue(line.ColorA);
    ui->nsBarFontBSB->setValue(line.ColorB);
    ui->nsBarFontGSB->setValue(line.ColorG);
    ui->nsBarFontRSB->setValue(line.ColorR);
    //
    QColor color = QColor::fromRgb(line.ColorR, line.ColorG, line.ColorB, line.ColorA);
    QString s("background: #"
              + QString(color.red() < 16 ? "0" : "") + QString::number(color.red(), 16)
              + QString(color.green() < 16 ? "0" : "") + QString::number(color.green(), 16)
              + QString(color.blue() < 16 ? "0" : "") + QString::number(color.blue(), 16) + ";");
    ui->chooseColorBtn->setStyleSheet(s);
    ui->nsBarFontSizeSB->setValue(line.Size);
    ui->nsBarFontBoldCB->setChecked(line.Bold);
    ui->nsBarFontItalicCB->setChecked(line.Italic);
    ui->nsBarContentCombo->setCurrentText(NetSpeedPluginMessages[line.ContentType]);
    ui->nsBarTagTxt->setText(QSTRING(line.Message));
    finishedLoading = true;
}

void PrefrencesWindow::on_chooseColorBtn_clicked()
{
    LOADINGCHECK
    QColorDialog d(QColor::fromRgb(CurrentBarLine.ColorR, CurrentBarLine.ColorG, CurrentBarLine.ColorB, CurrentBarLine.ColorA), this);
    d.exec();

    if (d.result() == QDialog::DialogCode::Accepted) {
        d.selectedColor().getRgb(&CurrentBarLine.ColorR, &CurrentBarLine.ColorG, &CurrentBarLine.ColorB, &CurrentBarLine.ColorA);
        ShowLineParameters(CurrentBarLine);
        SET_LINE_LIST_TEXT
    }
}

void PrefrencesWindow::on_nsBarTagTxt_textEdited(const QString &arg1)
{
    LOADINGCHECK
    CurrentBarLine.Message = arg1.toStdString();
    SET_LINE_LIST_TEXT
}

void PrefrencesWindow::on_nsBarContentCombo_currentIndexChanged(const QString &arg1)
{
    LOADINGCHECK
    CurrentBarLine.ContentType = NetSpeedPluginMessages.key(arg1);
    SET_LINE_LIST_TEXT
}

void PrefrencesWindow::on_applyNSBarSettingsBtn_clicked()
{
    auto conf = GetGlobalConfig();
    conf.speedBarConfig = CurrentConfig.speedBarConfig;
    SetGlobalConfig(conf);
}
