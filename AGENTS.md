# AGENTS.md — ATS LAB

## Projet
ATS LAB est un logiciel Delphi VCL Win32 de pilotage d'un récepteur ATS-25X2 / SI4735, accompagné d'un firmware ESP32.

## Environnement
- Delphi 12 Community Edition / VCL Win32
- La Community Edition interdit la compilation par DCC32 en ligne de commande :
  ne jamais tenter d'exécuter dcc32. Compiler uniquement depuis l'IDE Delphi.
- VCL
- Windows
- Firmware ESP32 sous Arduino IDE
- ESP32 classique avec Wi-Fi + Bluetooth Classic SPP
- SI4735 / ATS-25X2

## Règles absolues
1. Ne jamais modifier le décor, les images, positions ou dimensions du DFM sans demande explicite.
2. Ne jamais remplacer un TImage décoratif par un TPanel ou autre contrôle.
3. Ne jamais supprimer une unité, fonction, variable ou ressource avant d'avoir recherché toutes ses références.
4. Préserver les trois transports :
   - USB / port série
   - Bluetooth Classic SPP / port COM Windows
   - Wi-Fi TCP
5. Préserver le protocole de commandes ATS existant : PING, ID?, STATUS?, fréquence, modes, volume, squelch, BFO, scan, RDS, etc.
6. Préserver les corrections des bandes HAM, notamment la bande 40 m autour de 7.074 MHz.
7. Préserver About / identification firmware / RDS / NTP / Spectrum Analyzer.
8. Toute modification Delphi doit rester compatible Win32 / dcc32.
9. Toujours utiliser begin...end lorsqu'un if/else contrôle plus d'une instruction.
10. Ne pas effectuer de refactorisation cosmétique si elle augmente le risque de régression.

## Revue obligatoire avant modification
Avant de modifier du code :
- identifier les unités et méthodes concernées ;
- rechercher les appels et dépendances ;
- relever les effets possibles sur USB, Bluetooth et Wi-Fi ;
- vérifier les branches if/else et begin/end ;
- vérifier les ownership/free des objets VCL ;
- vérifier timers, threads et accès UI ;
- vérifier les conversions de fréquence Hz/kHz/MHz ;
- vérifier les limites de bandes.

## Après modification
Toujours :
- résumer précisément les fichiers modifiés ;
- expliquer pourquoi chaque changement est nécessaire ;
- signaler les risques éventuels ;
- rechercher les références cassées ;
- vérifier qu'aucune fonctionnalité existante n'a été supprimée ;
- ne pas tenter d'exécuter dcc32 dans cet environnement ;
- ne déclarer le projet Delphi "compilé" que si la compilation a réellement
  été lancée et réussie depuis l'IDE Delphi ;
- ne déclarer le firmware "compilé" que si Arduino IDE ou Arduino CLI a
  réellement terminé sans erreur.

## ATS LAB — règles UI
- Façade principale : conserver le style steampunk existant.
- Les éléments graphiques restent dans les DFM / ressources prévus.
- Pas de changement visuel implicite.
- Les fenêtres modales doivent rester séparées.
- La fréquence affichée côté PC est en kHz selon l'interface existante.
- Les actions distantes de la radio doivent pouvoir être reflétées côté PC.
- Les commandes locales ne doivent pas bloquer durablement les mises à jour distantes.

## Connexions
### Série / USB
Utiliser le transport série existant.

### Bluetooth
Bluetooth Classic SPP apparaît comme un port COM Windows.
Il doit réutiliser exactement le même protocole que le port série USB.

### Wi-Fi
Connexion TCP vers le serveur ATS sur le port prévu par le firmware.
Le protocole doit rester identique aux autres transports.

## Firmware ESP32
- Conserver USB, Wi-Fi et Bluetooth SPP.
- Bluetooth Classic augmente fortement la taille : utiliser une partition de type Huge APP (~3 MB) si nécessaire.
- Ne pas supprimer les prototypes explicites nécessaires à Arduino, notamment :
  - void FreqDispl();
  - void FreqDraw(float freq, int d);
  - void Segment(String freq, String mask, int d);
- Ne jamais supprimer une fonction d'affichage sous prétexte qu'elle semble non référencée sans vérifier le préprocesseur Arduino.
- Ne pas effacer toute la NVS ; pour le Wi-Fi, limiter les opérations au namespace prévu.
- Les diagnostics temporaires doivent être clairement identifiés et retirés seulement après validation.

## Bandes HAM
Éviter les fréquences exactement sur les frontières entre bandes broadcast et HAM.
Fréquences de départ conseillées :
- 160 m : 1.850 MHz
- 80 m : 3.630 MHz
- 40 m : 7.074 MHz
- 20 m : 14.074 MHz
- 17 m : 18.100 MHz
- 15 m : 21.074 MHz
- 12 m : 24.940 MHz
- 10 m : 28.500 MHz
- CB : 27.200 MHz

En cas de recouvrement de bandes dans le firmware :
1. conserver la bande courante si la fréquence reste dedans ;
2. sinon privilégier une bande HAM/non-AM avant une bande broadcast AM.

## Spectrum Analyzer
- Utiliser le mécanisme de scan natif ATS lorsque possible.
- Ne pas balayer artificiellement côté PC si SCANDATA? fournit déjà les données.
- Toute interaction avec la courbe doit préserver l'état de la radio et la connexion.

## Méthode de travail recommandée
### Autorisation de modification
- Une demande explicite d'action telle que « corrige », « ajoute », « modifie »,
  « optimise », « fais », « go » ou équivalent autorise directement les
  modifications nécessaires dans le périmètre demandé, sans demander une
  seconde confirmation.
- Avant d'écrire, effectuer la revue obligatoire décrite plus haut, puis
  modifier, vérifier et rendre compte du résultat dans le même travail.
- Une demande d'audit, de revue, d'analyse, de diagnostic ou d'explication
  seule reste strictement en lecture seule. Ne rien modifier tant que
  l'utilisateur n'a pas ensuite demandé explicitement une correction.
- Une autorisation de modifier le code n'autorise pas automatiquement un
  commit, un push, une publication, un tag, un téléversement sur un appareil
  ou une réécriture de l'historique Git. Ces actions exigent toujours une
  demande explicite.
- Si une action est destructive, dépasse clairement le périmètre demandé ou
  impose un choix fonctionnel important impossible à déduire, demander une
  confirmation ciblée.

Pour une demande de correction ou d'ajout explicitement autorisée :
1. Analyser les unités, appels, dépendances et risques concernés.
2. Effectuer directement les modifications nécessaires.
3. Vérifier les références, les régressions et compiler/tester lorsque les
   outils correspondants sont réellement disponibles.
4. Après modification, fournir un diff ou un résumé exact.

Pour une revue de code :
- ne modifier aucun fichier ;
- classer les problèmes par gravité : bloquant, majeur, moyen, mineur ;
- donner fichier + méthode + explication ;
- éviter les faux positifs ;
- vérifier particulièrement les begin/end Delphi et la logique de synchronisation distante.

## Git
- Ne jamais réécrire l'historique Git sans demande explicite.
- Faire des commits petits et descriptifs.
- Avant gros changement, recommander un commit de sauvegarde.
