# Clipper — démarrage rapide (version de test 0.2)

**Clipper** est un firmware de **synthétiseur** pour le M-VAVE FM-1 : un seul instrument, comme le firmware d'origine, Felucca ou Baudgirl. Ce n'est plus une groovebox : pas de pistes, pas de boîte à rythmes, pas de mode song, pas de samples intégrés. Il garde de SLOOP les 12 moteurs, les effets, l'arpégiateur, le séquenceur de pas, l'audio USB et le MIDI. Il ajoute **HYBRID**, une voix à 3 oscillateurs, 2 filtres et une matrice de modulation de 16 slots, avec 24 sons d'usine. Il ajoute aussi le pitch bend, la molette, l'aftertouch et la pédale de sustain sur tous les moteurs.

> **C'est une première version de test** : elle n'a encore jamais tourné sur une vraie FM-1. Avant d'installer, fais une **sauvegarde** dans l'éditeur SLOOP (Projects → Backup → *Save a backup*).

## Un dossier à part

Clipper vit dans son propre dossier, **`CLIPPER`**, à côté de celui de SLOOP. Rien n'est partagé entre les deux :

| | SLOOP | Clipper |
| --- | --- | --- |
| Lancer l'installation | `INSTALL-SLOOP.bat` | `INSTALL-CLIPPER.bat` |
| Ouvrir l'éditeur | `OPEN-EDITOR.bat` | `OPEN-EDITOR.bat` (du dossier CLIPPER) |
| Adresse | http://localhost:**8766** | http://localhost:**8767** |
| Écran de démarrage | voilier, « sloop » | trois-mâts, « clipper » |

Sur la FM-1, Clipper se reconnaît à son écran de démarrage et à HOME (maintenu) → SYSTEM → ABOUT, qui affiche **CLIPPER 0.2**.

## 1. Compiler et installer

1. Ferme la fenêtre de SLOOP si elle tourne encore. Ce n'est pas obligatoire, mais c'est plus clair.
2. Double-clique sur **`INSTALL-CLIPPER.bat`**. Il utilise les mêmes outils que SLOOP (WSL MinUI-Build, la chaîne JieLi). La première fois, il copie les trois fichiers du SDK depuis `..\SLOOP\build\deps\ac79`. S'il ne les trouve pas, copie ce dossier `build\deps\ac79` de SLOOP dans `CLIPPER\build\deps\`.
3. La page d'installation s'ouvre sur http://localhost:8767/webapp/installer/ (Chrome ou Edge, FM-1 en USB). Clique sur **Install**.
4. La FM-1 redémarre sur l'écran **clipper**. Tes projets, presets, samples et réglages de SLOOP sont toujours là.

## Nouveau dans la 0.2 : le son

- **Ladder MOOG** : un filtre 4 pôles 24 dB à retour sans délai, avec une saturation tanh à l'entrée. Il chauffe quand on le pousse, sa résonance sature en douceur et il auto-oscille juste. C'est un type de filtre dans HYBRID. Sur ANALOG : FX → FILTER → **MODL** (SVF 12 dB ou MOOG 24 dB).
- **Résonance douce** : les filtres SVF d'HYBRID saturent en douceur au lieu d'écrêter.
- **Anti-aliasing** : triangles à bande limitée, hard sync corrigé à chaque reset, FM, feedback et wavefolder tenus sous 20 kHz dans les aigus.
- **Réverb plate** (d'après Dattorro) : FX → REVERB : TYPE (ROOM ou PLATE), SIZE, DAMP et PRE (pré-délai en ms). Le chorus a sa propre page CHORUS.
- **WAVE, tables d'ondes** : 16 tables de 16 frames (formes classiques, PWM, sync, FM, fold, formants, balayages façon PPG, orgue, chip, comb…). POS les parcourt, et l'enveloppe et le LFO le font bouger (ENV DEST et LFO DEST → SHP). DTN ajoute un 2e oscillateur, SUB un sinus à l'octave du dessous, puis le filtre d'ANALOG (SVF ou MOOG). Sans repliement. Tes propres tables (WAV façon Serum) s'envoient dans un emplacement USR depuis l'onglet Samples de l'éditeur, mode WAVETABLE ; TABL = USR1 à USR4 les joue.
- **Plus de samples intégrés** : 300 Ko de flash libérés pour la synthèse. SAMPLE et GRAIN jouent tes propres samples (onglet Samples de l'éditeur, 4 emplacements). 181 sons d'usine : cinq nouveaux sur le ladder (MOOG BASS, MOOG LEAD, LADDER PAD, MINI BASS, MINI LEAD) et seize sur WAVE. 14 moteurs.

## 2. Sur la FM-1

| | |
| --- | --- |
| **HOME** | L'écran du synthé : le nom du son, sa famille, son numéro dans la liste, le moteur et un oscilloscope. Les 4 knobs règlent les contrôles principaux du son. Appuie encore sur HOME pour le visualiseur plein écran. |
| **PRESETS** | Tous les sons, un par un. |
| **ALGORITHM** | Saute à la famille suivante (PAD, BASS, LEAD, PLCK, KEYS, STAB, FX…). |
| **FX, EDIT, ARP, GLO, SAVE** | Un appui ouvre les pages. Bouton maintenu, le clavier joue toujours. Pages FX : FX, FILTER (avec MODL), DLY, REVERB, CHORUS. |
| **SEQ, SEL** | Maintenus : SEQ montre les 16 pas sur les touches, SEL la tonalité et les accords. Un appui ouvre leurs pages. |
| **REC** | Enregistre ce que tu joues dans le pattern. REC maintenu efface le pattern. |
| **MIDI** | Omni : tous les canaux jouent le synthé. |

Un projet de SLOOP se charge avec seulement sa première piste.

## 3. Jouer HYBRID

- Tourne **PRESETS** (ou **ALGORITHM** pour changer de famille). Les sons HYBRID sont rangés dans les familles : **CLIPPER**, POLY PAD, STRINGS, VOX, GLASS PAD et DRONE (PAD) ; ACID, WOBBLE, REESE, FOLD BASS et 808 SUB (BASS) ; SYNC LEAD, HOOVER, CHIP LEAD et TRANCE (LEAD) ; PLUCK, SAW PLUCK, FM BELL, SPACE BEL et METAL (PLCK) ; FM KEYS (KEYS) ; BRASS (STAB) ; RISER et INIT (FX).
- Sur **HOME**, les 4 knobs règlent **CUT**, **RES**, **M1** et **M2**.
- Page **EDIT** : CUT, RES, FENV (décalages sur les deux filtres), **PTCH** (le patch d'usine), et **M1 à M4**, quatre macros dont chaque patch choisit l'effet. Par exemple, sur CLIPPER, M1 ouvre le filtre et M2 élargit les supersaws.
- Avec un clavier MIDI : pitch bend, molette (vibrato sur plusieurs patchs), aftertouch, sustain, et M1 à M4 sur les CC 16 à 19.

## 4. Éditer tout le patch dans l'éditeur

Double-clique sur `OPEN-EDITOR.bat` (http://localhost:8767/webapp/editor/), clique sur **Connect**, puis choisis un son HYBRID. Les pages Tracks, Song et Drum kit n'apparaissent plus avec Clipper. Dans l'onglet **Sound**, le panneau **HYBRID patch** montre tout : oscillateurs, filtres et routage, ENV2 et ENV3, LFO2 et LFO3 (synchro au tempo possible), la matrice de 16 slots et le bend. Avec **live** coché, chaque réglage s'entend tout de suite.

Les autres boutons du panneau :
- **Export patch** et **Import patch** gardent tes sons dans des fichiers `.clipper.json`.
- **factory** + **load** recharge un patch d'usine.

Un projet garde le PTCH, mais pas le patch modifié : exporte-le pour le garder.

Pour essayer l'éditeur sans la FM-1, ouvre `web/editor.html?mock=1` dans Chrome.

## 5. Le test dont j'ai besoin : la charge processeur

C'est ce qui décide combien de voix Clipper peut jouer. La FM-1 doit être branchée en USB à l'ordinateur, sinon la case CPU affiche l'état USB à la place.

1. Choisis le son **CLIPPER** avec PRESETS.
2. Va sur **GLO → SYSTEM** : la 4e case affiche **CPU %**.
3. Tiens **8 notes** (les deux mains, ou un accord depuis un clavier MIDI) et note le chiffre.
4. Refais la même chose avec **POLY PAD** puis **TRANCE**.
5. Si tout reste sous ~70 %, ferme la fenêtre, puis lance **`INSTALL-CLIPPER.bat 12`**. Ça compile la version à **12 voix**. Réinstalle et refais la mesure avec 12 notes.

Envoie-moi les chiffres. Signale aussi un son qui craque, une note qui reste coincée ou un écran bizarre.

Mes estimations, mesurées sur ordinateur : une voix typique coûte environ 2,5 % du processeur, et CLIPPER (le plus lourd) environ 6,6 %, donc environ 53 % pour 8 voix.

## Revenir en arrière

- **Vers SLOOP** : lance `INSTALL-SLOOP.bat` depuis le dossier de SLOOP. Un son réglé sur HYBRID y jouera un autre moteur.
- **Vers le firmware officiel** : passe par la page d'installation (« back to the official V15 »), après avoir fait une sauvegarde.
