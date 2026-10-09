# Clipper — démarrage rapide (version de test 0.1)

**Clipper** est le firmware « synthé » pour le M-VAVE FM-1. Il vient de SLOOP et garde tout ce que SLOOP sait faire. Il ajoute **HYBRID**, une voix à 3 oscillateurs, 2 filtres et une matrice de modulation de 16 slots, avec 24 sons d'usine. Il ajoute aussi le pitch bend, la molette, l'aftertouch et la pédale de sustain sur tous les moteurs.

> **C'est une première version de test** : elle n'a encore jamais tourné sur une vraie FM-1. Avant d'installer, fais une **sauvegarde** dans l'éditeur SLOOP (Projects → Backup → *Save a backup*).

## Un dossier à part

Clipper vit dans son propre dossier, **`clipper-fm1`**, à côté de celui de SLOOP. Rien n'est partagé entre les deux :

| | SLOOP | Clipper |
| --- | --- | --- |
| Lancer l'installation | `INSTALL-SLOOP.bat` | `INSTALL-CLIPPER.bat` |
| Ouvrir l'éditeur | `OPEN-EDITOR.bat` | `OPEN-EDITOR.bat` (du dossier clipper-fm1) |
| Adresse | http://localhost:**8766** | http://localhost:**8767** |
| Écran de démarrage | voilier, « sloop » | trois-mâts, « clipper » |

Sur la FM-1, Clipper se reconnaît à son écran de démarrage et à HOME (maintenu) → SYSTEM → ABOUT, qui affiche **CLIPPER 0.1**.

## 1. Compiler et installer

1. Ferme la fenêtre de SLOOP si elle tourne encore. Ce n'est pas obligatoire, mais c'est plus clair.
2. Double-clique sur **`INSTALL-CLIPPER.bat`**. Il utilise les mêmes outils que SLOOP (WSL MinUI-Build, la chaîne JieLi). La première fois, il copie les trois fichiers du SDK depuis `..\sloop-fm1\build\deps\ac79`. S'il ne les trouve pas, copie ce dossier `build\deps\ac79` de SLOOP dans `clipper-fm1\build\deps\`.
3. La page d'installation s'ouvre sur http://localhost:8767/webapp/installer/ (Chrome ou Edge, FM-1 en USB). Clique sur **Install**.
4. La FM-1 redémarre sur l'écran **clipper**. Tes projets, presets, samples et réglages de SLOOP sont toujours là.

## 2. Jouer HYBRID

- Sur une piste de synthé (1, 2 ou 3), tourne **PRESETS**. Les sons HYBRID sont rangés dans les familles : **CLIPPER**, POLY PAD, STRINGS, VOX, GLASS PAD et DRONE (PAD) ; ACID, WOBBLE, REESE, FOLD BASS et 808 SUB (BASS) ; SYNC LEAD, HOOVER, CHIP LEAD et TRANCE (LEAD) ; PLUCK, SAW PLUCK, FM BELL, SPACE BEL et METAL (PLCK) ; FM KEYS (KEYS) ; BRASS (STAB) ; RISER et INIT (FX).
- Sur **HOME**, les 4 knobs règlent **CUT**, **RES**, **M1** et **M2**.
- Page **EDIT** : CUT, RES, FENV (décalages sur les deux filtres), **PTCH** (le patch d'usine), et **M1 à M4**, quatre macros dont chaque patch choisit l'effet. Par exemple, sur CLIPPER, M1 ouvre le filtre et M2 élargit les supersaws.
- Avec un clavier MIDI : pitch bend, molette (vibrato sur plusieurs patchs), aftertouch, sustain, et M1 à M4 sur les CC 16 à 19.

## 3. Éditer tout le patch dans l'éditeur

Double-clique sur `OPEN-EDITOR.bat` (http://localhost:8767/webapp/editor/), clique sur **Connect**, puis choisis une piste qui joue HYBRID. Dans l'onglet **Sound**, le panneau **HYBRID patch** montre tout : oscillateurs, filtres et routage, ENV2 et ENV3, LFO2 et LFO3 (synchro au tempo possible), la matrice de 16 slots et le bend. Avec **live** coché, chaque réglage s'entend tout de suite.

Les autres boutons du panneau :
- **Export patch** et **Import patch** gardent tes sons dans des fichiers `.clipper.json`.
- **factory** + **load** recharge un patch d'usine.

Un projet garde le PTCH, mais pas le patch modifié : exporte-le pour le garder.

Pour essayer l'éditeur sans la FM-1, ouvre `web/editor.html?mock=1` dans Chrome.

## 4. Le test dont j'ai besoin : la charge processeur

C'est ce qui décide combien de voix Clipper peut jouer. La FM-1 doit être branchée en USB à l'ordinateur, sinon la case CPU affiche l'état USB à la place.

1. Mets le son **CLIPPER** sur la piste 1 et coupe les autres pistes.
2. Va sur **GLO → SYSTEM** : la 4e case affiche **CPU %**.
3. Tiens **8 notes** (les deux mains, ou un accord depuis un clavier MIDI) et note le chiffre.
4. Refais la même chose avec **POLY PAD** puis **TRANCE**.
5. Si tout reste sous ~70 %, ferme la fenêtre, puis lance **`INSTALL-CLIPPER.bat 12`**. Ça compile la version à **12 voix**. Réinstalle et refais la mesure avec 12 notes.

Envoie-moi les chiffres. Signale aussi un son qui craque, une note qui reste coincée ou un écran bizarre.

Mes estimations, mesurées sur ordinateur : une voix typique coûte environ 2,5 % du processeur, et CLIPPER (le plus lourd) environ 6,6 %, donc environ 53 % pour 8 voix.

## Revenir en arrière

- **Vers SLOOP** : lance `INSTALL-SLOOP.bat` depuis le dossier de SLOOP. Une piste réglée sur HYBRID y jouera un autre moteur.
- **Vers le firmware officiel** : passe par la page d'installation (« back to the official V15 »), après avoir fait une sauvegarde.
