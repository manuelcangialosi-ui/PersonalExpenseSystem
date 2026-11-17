# Sistema di Gestione delle Spese Personali e del Budget

Applicazione console sviluppata in C++ con database relazionale SQLite per registrare le spese giornaliere, suddividerle per categoria, impostare budget mensili e consultare report riepilogativi.

Autore: Manuel Cangialosi

## Struttura del progetto

- src/main.cpp: codice sorgente completo dell'applicazione console.
- sql/database.sql: script SQL per la creazione delle tabelle, dei vincoli di integrità e dei dati di esempio.
- demo/demo_video.mp4: video dimostrativo delle funzionalità principali.

## Requisiti per l'esecuzione

- Compilatore: compilatore C++ con supporto allo standard C++17 (es. g++/ GCC o Clang).
- Database e librerie: SQLite3 (libsqlite3-dev / sqlite3).
- Librerie standard C++ utilizzate: <iostream>, <iomanip>, <string>, <cctype>, oltre all'header <sqlite3.h>.

## Istruzioni di compilazione e avvio

Aprire il terminale nella cartella principale del progetto (PersonalExpenseSystem) ed eseguire i seguenti passaggi:

1. popolare il database con i dati di esempio:
   bash
   sqlite3 spese_personali.db < sql/database.sql