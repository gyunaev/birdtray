/*
 * Qt Installer script for a non-interactive installation of Qt5 on Windows.
 * Installs the 64-bit package if environment variable PLATFORM="x64".
 */

/* globals QInstaller, QMessageBox, buttons, gui, installer, console */

// Look for Name elements in
// https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt5_5120/Updates.xml
// Unfortunately it is not possible to disable deps like qt.tools.qtcreator
var INSTALL_COMPONENTS = [
    installer.environmentVariable("PLATFORM") == "x64" ?
    "win64_msvc2017_64" :
    "win32_msvc2017",
];

function Controller() {
    // Continue on installing to an existing (possibly empty) directory.
    installer.setMessageBoxAutomaticAnswer("OverwriteTargetDirectory", QMessageBox.Yes);
    // Continue at "SHOW FINISHED PAGE"
    installer.installationFinished.connect(function() {
        console.log("installationFinished");
        gui.clickButton(buttons.NextButton);
    });
}

Controller.prototype.WelcomePageCallback = function() {
    console.log("Step: " + gui.currentPageWidget());
    // At least for 3.0.4 immediately clicking Next fails, so wait a bit.
    // https://github.com/benlau/qtci/commit/85cb986b66af4807a928c70e13d82d00dc26ebf0
    gui.clickButton(buttons.NextButton, 1000);
};

Controller.prototype.CredentialsPageCallback = function() {
    console.log("Step: " + gui.currentPageWidget());
    gui.clickButton(buttons.NextButton);
};

Controller.prototype.DynamicTelemetryPluginFormCallback = function() {
    var widget = gui.currentPageWidget();
    widget.TelemetryPluginForm.statisticGroupBox.disableStatisticRadioButton.checked = true;
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.IntroductionPageCallback = function() {
    console.log("Step: " + gui.currentPageWidget());
    gui.clickButton(buttons.NextButton);
};

Controller.prototype.TargetDirectoryPageCallback = function() {
    console.log("Step: " + gui.currentPageWidget());
    var qtHome = installer.environmentVariable("QT_HOME");
    gui.currentPageWidget().TargetDirectoryLineEdit.setText(qtHome);
    gui.clickButton(buttons.NextButton);
};

Controller.prototype.ComponentSelectionPageCallback = function() {
    console.log("Step: " + gui.currentPageWidget());
    var page = gui.currentPageWidget();
    var checkBox = gui.findChild(page, "Latest releases");
    var fetchButton = gui.findChild(page, "FetchCategoryButton");

    checkBox.click();
    fetchButton.click();

    page.deselectAll();
    var latestVersion = installer.environmentVariable("Q_LATEST_VERSION");
    var latestVersionMajor = latestVersion[0];
    var prefix = "qt.qt" + latestVersion[0] + "." + latestVersion + ".";
    for (var i = 0; i < INSTALL_COMPONENTS.length; i++) {
        page.selectComponent(prefix + INSTALL_COMPONENTS[i]);
    }
    gui.clickButton(buttons.NextButton);
};

Controller.prototype.LicenseAgreementPageCallback = function() {
    console.log("Step: " + gui.currentPageWidget());
    gui.currentPageWidget().AcceptLicenseRadioButton.setChecked(true);
    gui.clickButton(buttons.NextButton);
};

Controller.prototype.StartMenuDirectoryPageCallback = function() {
    console.log("Step: " + gui.currentPageWidget());
    gui.clickButton(buttons.NextButton);
};

Controller.prototype.ReadyForInstallationPageCallback = function() {
    console.log("Step: " + gui.currentPageWidget());
    gui.clickButton(buttons.NextButton);
};

Controller.prototype.FinishedPageCallback = function() {
    console.log("Step: " + gui.currentPageWidget());
    // TODO somehow the installer crashes after this step.
    // https://stackoverflow.com/questions/25105269/silent-install-qt-run-installer-on-ubuntu-server
    var checkBoxForm = gui.currentPageWidget().LaunchQtCreatorCheckBoxForm;
    if (checkBoxForm && checkBoxForm.launchQtCreatorCheckBox) {
        checkBoxForm.launchQtCreatorCheckBox.checked = false;
    }
    gui.clickButton(buttons.FinishButton);
    console.log("Finished Qt installation");
};
