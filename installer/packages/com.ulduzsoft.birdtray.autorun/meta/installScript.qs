function Component() {
    // constructor
    if (installer.isInstaller()) {
        installer.setValue("AllUsers", true);
    }
}

Component.prototype.createOperations = function() {
    component.createOperations();
    try {
        var reg = installer.environmentVariable("SystemRoot") + "\\System32\\reg.exe";
        var cmd = installer.environmentVariable("SystemRoot") + "\\System32\\cmd.exe";

        component.addOperation(
            "Execute", reg, "add", "HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                "/v", "BirdTray", "/t", "REG_SZ", "/d",
                 installer.value("TargetDir") + "\\birdtray.exe",
            "UNDOEXECUTE", reg, "delete", "HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                "/f", "/v", "BirdTray");
    } catch (e) {
        print(e);
    }
}
