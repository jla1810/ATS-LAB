# ATS LAB

ATS LAB est une application Windows au style steampunk permettant de piloter un récepteur ATS-25X2 / SI4735 depuis un PC. Le dépôt contient l’application Delphi VCL Win32 et le firmware ESP32 associé.

## État du projet

- branche de développement : `develop` ;
- dernière base publiée : tag `v1.1.2` ;
- version actuellement affichée par l’application : `1.1.1` ;
- firmware : ATS LAB V6.0 pour ESP32 classique.

La branche `develop` contient également les évolutions postérieures au tag `v1.1.2`, notamment la bande CB et l’affichage numérique des réglages rotatifs.

## Fonctions principales

- connexion USB par port série ;
- connexion Bluetooth Classic SPP par port COM Windows ;
- connexion Wi-Fi/TCP ;
- protocole ATS commun aux trois transports ;
- synchronisation de la fréquence, du mode et des réglages du récepteur ;
- modes AM, FM, USB, LSB, CW et CW-R selon la bande ;
- bandes radio LW, MW, SW, AIR, FM et VHF ;
- bandes HAM 160 m, 80 m, 40 m, 20 m, 17 m, 15 m, 12 m et 10 m ;
- bande CB sur 120 positions : 40 canaux INF, 40 CEPT et 40 SUP ;
- affichage Nixie de la fréquence ;
- affichage direct des valeurs Volume, Squelch, BFO, RF Gain, AF Gain et Clarifier ;
- S-mètre, RDS, mémoires, favoris et saisie directe d’une fréquence ;
- Spectrum Analyzer avec scan ATS natif, détection des stations et export CSV/PNG ;
- configuration Wi-Fi, identification du firmware et synchronisation NTP ;
- diagnostic de connexion par `F2` : transport, point de connexion, version du firmware, compteurs TX/RX, PING/PONG, latence et derniers échanges.

## Matériel et environnement

- Windows ;
- Delphi 12 Community Edition ;
- VCL Win32 ;
- récepteur ATS-25X2 basé sur le SI4735 ;
- ESP32 classique avec Wi-Fi et Bluetooth Classic SPP ;
- Arduino IDE pour le firmware.

## Compilation de l’application

1. Ouvrir `ATSLab_v0070.dproj` dans Delphi 12 Community Edition.
2. Sélectionner la plateforme **Win32**.
3. Choisir la configuration Debug ou Release.
4. Compiler depuis l’IDE Delphi.

La Community Edition ne permet pas d’utiliser `dcc32` directement en ligne de commande. Les ressources du dossier `Data` doivent conserver leur arborescence et rester accessibles à côté de l’exécutable.

## Compilation du firmware

Le firmware suivi par Git se trouve dans :

`SI4735_2.8_TFT_WIFI_V6.0/SI4735_2.8_TFT_WIFI_V6.0.ino`

L’ESP32 classique doit disposer de Wi-Fi et de Bluetooth Classic. Bluetooth SPP augmente sensiblement la taille du programme ; sélectionner une partition de type **Huge APP (~3 MB)** si nécessaire.

Compiler et téléverser le firmware depuis Arduino IDE. La configuration exacte de la carte, de l’écran TFT et du SI4735 doit correspondre au récepteur utilisé.

## Connexions

### USB

Sélectionner le port COM du récepteur. L’application utilise le transport série existant et le protocole ATS.

### Bluetooth

Associer d’abord l’ESP32 à Windows. Le service Bluetooth Classic SPP apparaît ensuite comme un port COM et réutilise exactement le même protocole que l’USB.

### Wi-Fi

Configurer les identifiants Wi-Fi depuis l’application, puis utiliser la connexion TCP vers l’adresse du récepteur. Le firmware fournit les commandes d’identification, d’état et de contrôle utilisées par ATS LAB.

## Spectrum Analyzer et RDS

Le Spectrum Analyzer exploite les données du mécanisme de scan ATS. Il peut détecter les pics, lister les stations, accorder la fréquence sélectionnée et exporter les résultats.

Le RDS est utilisé en FM broadcast. Pendant un scan, son état est géré afin de ne pas perturber la réception des données de spectre, puis restauré à la fin du scan.

## Bande CB

La bande CB démarre sur 27,185 MHz et propose trois séries de 40 canaux :

- `INF` : canaux inférieurs ;
- `CEPT` : canaux standards ;
- `SUP` : canaux supérieurs.

La façade affiche la série et le numéro du canal sélectionné.

## Organisation du dépôt

- `MainUnit.*` : façade principale et logique de pilotage ;
- `uATSConnection.pas` : transports série et TCP ;
- `uATSProtocol.pas` : commandes et réponses ATS ;
- `SpectrumUnit.*` : analyseur de spectre ;
- `SerialConnectUnit.*` : sélection et configuration des connexions ;
- `Data/` : décors, boutons, Nixies, lampes et autres ressources graphiques ;
- `SI4735_2.8_TFT_WIFI_V6.0/` : firmware ESP32 suivi par Git.

## Branches et versions

- `master` : versions stables validées ;
- `develop` : développements et validations en cours ;
- tags `v1.x` : jalons de publication récents ;
- tags `v0.09.1d-*` : historique des versions Spectrum et RDS.

## Précautions

- conserver les trois transports USB, Bluetooth SPP et Wi-Fi TCP ;
- ne pas modifier l’arborescence de `Data` ;
- éviter les fréquences situées exactement sur une frontière de bande ;
- préserver le protocole entre l’application et le firmware lors de toute évolution.
