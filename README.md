# ATS LAB

ATS LAB est une application Delphi VCL Win32 destinée au pilotage d'un
récepteur ATS-25X2 / SI4735.

## Version stable

La base stable actuelle est **ATS LAB v0.09.1d NIXIE DOT**.

Tags disponibles :

- `v0.09.1d` : première base stable enregistrée dans Git ;
- `v0.09.1d-optimized` : base stable avec optimisations validées ;
- `v0.09.1d-scan-rds-fix` : correction validée du scan FM et du RDS.

## Fonctions principales

- connexion USB par port série ;
- Bluetooth Classic SPP par port COM Windows ;
- connexion Wi-Fi/TCP ;
- synchronisation de la fréquence, du mode et des réglages ATS ;
- bandes HAM et bandes radio LW, MW, SW et FM ;
- affichage de fréquence sur huit Nixies avec point décimal indépendant ;
- Spectrum Analyzer utilisant les commandes de scan ATS ;
- affichage RSSI/S-mètre, RDS et informations du récepteur.

Le Spectrum Analyzer et le RDS sont réservés à la bande FM broadcast de
87,5 à 108 MHz. Le RDS est temporairement désactivé pendant un scan puis
réactivé après `SCANEND`.

## Environnement

- Delphi 11 Alexandria ;
- cible VCL Win32 (`dcc32`) ;
- Windows ;
- récepteur ATS-25X2 / SI4735 ;
- ESP32 classique pour les fonctions Wi-Fi et Bluetooth Classic SPP.

## Compilation

Ouvrir `ATSLab_v0070.dproj` dans Delphi 11, sélectionner la cible Win32 puis
compiler depuis l'IDE.

Les fichiers générés (`.dcu`, `.exe`, fichiers locaux de l'IDE et paramètres
INI) ne sont pas suivis par Git.

## Connexions

### USB et Bluetooth

USB et Bluetooth Classic SPP utilisent le même transport série et le même
protocole ATS. Le port SPP doit être associé à un port COM Windows.

### Wi-Fi

La connexion Wi-Fi utilise TCP vers l'adresse et le port configurés pour le
serveur ATS.

## Branches

- `master` : version stable validée ;
- `develop` : développement et validations avant intégration dans `master`.

## Ressources

Les images nécessaires à l'interface sont conservées dans `Data`. Leur
arborescence doit rester placée à côté de l'exécutable lors de l'utilisation
du programme.

