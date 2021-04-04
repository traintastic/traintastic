; Inno Setup Script for Traintastic

#define Name "Traintastic"
#define Version "0.0.1"
#ifndef Codename
  #define Codename "master"
#endif
#define Publisher "Reinder Feenstra"
#define URL "https://traintastic.org"
#define ServerExeName "traintastic-server.exe"
#define ClientExeName "traintastic-client.exe"

[Setup]
AppId={{7E509202-257F-4859-B8FA-D87D636342BB}
AppName={#Name}
AppVersion={#Version}
AppVerName={#Name} v{#Version} {#Codename}
AppPublisher={#Publisher}
AppPublisherURL={#URL}
AppSupportURL={#URL}
AppUpdatesURL={#URL}
DefaultDirName={autopf}\{#Name}
DisableDirPage=yes
DisableProgramGroupPage=yes
LicenseFile=..\..\LICENSE
OutputDir=output
OutputBaseFilename=traintastic-setup-{#Version}-{#Codename}
SolidCompression=yes
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "dutch"; MessagesFile: "compiler:Languages\Dutch.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"

[Types]
Name: "clientandserver"; Description: "Client and Server"
Name: "clientonly"; Description: "Client only"

[Components]
Name: "server"; Description: "Traintastic server"; Types: clientandserver
Name: "client"; Description: "Traintastic client"; Types: clientandserver clientonly; Flags: fixed

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; Server
Source: "..\..\build-server\Release\{#ServerExeName}"; DestDir: "{app}\server"; Flags: ignoreversion; Components: server
Source: "..\..\server\thirdparty\lua5.3\bin\win64\lua53.dll"; DestDir: "{app}\server"; Flags: ignoreversion; Components: server
; Client
Source: "..\..\build-client\{#ClientExeName}"; DestDir: "{app}\client"; Flags: ignoreversion; Components: client
Source: "..\..\build-client\*.dll"; DestDir: "{app}\client"; Flags: ignoreversion; Components: client
Source: "..\..\build-client\bearer\*.dll"; DestDir: "{app}\client\bearer"; Flags: ignoreversion; Components: client
Source: "..\..\build-client\iconengines\*.dll"; DestDir: "{app}\client\iconengines"; Flags: ignoreversion; Components: client
Source: "..\..\build-client\imageformats\*.dll"; DestDir: "{app}\client\imageformats"; Flags: ignoreversion; Components: client
Source: "..\..\build-client\platforms\*.dll"; DestDir: "{app}\client\platforms"; Flags: ignoreversion; Components: client
Source: "..\..\build-client\styles\*.dll"; DestDir: "{app}\client\styles"; Flags: ignoreversion; Components: client
Source: "..\..\lang\*.txt"; DestDir: "{commonappdata}\traintastic\client\lang"; Flags: ignoreversion; Components: client

[Icons]
; Server
Name: "{autoprograms}\{#Name}\{#Name} server"; Filename: "{app}\server\{#ServerExeName}"; Components: server
Name: "{autodesktop}\{#Name} server"; Filename: "{app}\server\{#ServerExeName}"; Tasks: desktopicon; Components: server
; Client
Name: "{autoprograms}\{#Name}\{#Name} client"; Filename: "{app}\client\{#ClientExeName}"; Components: client
Name: "{autodesktop}\{#Name} client"; Filename: "{app}\client\{#ClientExeName}"; Tasks: desktopicon; Components: client

