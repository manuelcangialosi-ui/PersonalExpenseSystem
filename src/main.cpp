/*
 * Progetto Finale di Programmazione
 * Sistema di Gestione delle Spese Personali e del Budget
 * File: src/main.cpp
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <cctype>
#include <sqlite3.h>

using namespace std;

const string NOME_DB = "spese_personali.db";

// Rimuove eventuali spazi all'inizio e alla fine della stringa
string pulisciSpazi(const string& s) {
    size_t inizio = s.find_first_not_of(" \t\r\n");
    if (inizio == string::npos) return "";
    size_t fine = s.find_last_not_of(" \t\r\n");
    return s.substr(inizio, fine - inizio + 1);
}

// Controlla che la data sia nel formato YYYY-MM-DD
bool dataValida(const string& data) {
    if (data.size() != 10 || data[4] != '-' || data[7] != '-') return false;
    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) continue;
        if (!isdigit((unsigned char)data[i])) return false;
    }
    int mese = stoi(data.substr(5, 2));
    int giorno = stoi(data.substr(8, 2));
    return (mese >= 1 && mese <= 12 && giorno >= 1 && giorno <= 31);
}

// Controlla che il mese sia nel formato YYYY-MM
bool meseValido(const string& meseStr) {
    if (meseStr.size() != 7 || meseStr[4] != '-') return false;
    for (int i = 0; i < 7; i++) {
        if (i == 4) continue;
        if (!isdigit((unsigned char)meseStr[i])) return false;
    }
    int mese = stoi(meseStr.substr(5, 2));
    return (mese >= 1 && mese <= 12);
}

// Crea le tabelle se non esistono ancora e abilita le chiavi esterne
bool preparaDatabase(sqlite3* db) {
    char* err = nullptr;

    if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &err) != SQLITE_OK) {
        cerr << "Errore foreign keys: " << err << endl;
        sqlite3_free(err);
        return false;
    }

    const char* queryTabelle =
        "CREATE TABLE IF NOT EXISTS categorie ("
        "    id_categoria INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    nome VARCHAR(100) NOT NULL UNIQUE,"
        "    CHECK (LENGTH(TRIM(nome)) > 0)"
        ");"
        "CREATE TABLE IF NOT EXISTS spese ("
        "    id_spesa INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    data_spesa DATE NOT NULL,"
        "    importo DECIMAL(10, 2) NOT NULL,"
        "    id_categoria INTEGER NOT NULL,"
        "    descrizione TEXT,"
        "    CHECK (LENGTH(data_spesa) = 10),"
        "    CHECK (importo > 0),"
        "    FOREIGN KEY (id_categoria) REFERENCES categorie(id_categoria)"
        "        ON DELETE RESTRICT ON UPDATE CASCADE"
        ");"
        "CREATE TABLE IF NOT EXISTS budget_mensili ("
        "    id_budget INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    mese CHAR(7) NOT NULL,"
        "    id_categoria INTEGER NOT NULL,"
        "    importo_budget DECIMAL(10, 2) NOT NULL,"
        "    CHECK (LENGTH(mese) = 7),"
        "    CHECK (importo_budget > 0),"
        "    UNIQUE (mese, id_categoria),"
        "    FOREIGN KEY (id_categoria) REFERENCES categorie(id_categoria)"
        "        ON DELETE CASCADE ON UPDATE CASCADE"
        ");";

    if (sqlite3_exec(db, queryTabelle, nullptr, nullptr, &err) != SQLITE_OK) {
        cerr << "Errore inizializzazione tabelle: " << err << endl;
        sqlite3_free(err);
        return false;
    }

    return true;
}

// Restituisce l'id della categoria se presente nel DB, altrimenti -1
int cercaCategoria(sqlite3* db, const string& nome) {
    const char* sql = "SELECT id_categoria FROM categorie WHERE LOWER(nome) = LOWER(?);";
    sqlite3_stmt* stmt = nullptr;
    int id = -1;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, nome.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            id = sqlite3_column_int(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
    return id;
}

// Modulo 1: aggiunta di una nuova categoria di spesa
void gestioneCategorie(sqlite3* db) {
    string nome;
    cout << "\n--- Gestione Categorie ---\n";
    cout << "Nome della categoria: ";
    getline(cin, nome);
    nome = pulisciSpazi(nome);

    if (nome.empty()) {
        cout << "Errore: il nome della categoria non puo' essere vuoto.\n";
        return;
    }

    if (cercaCategoria(db, nome) != -1) {
        cout << "La categoria esiste gia'.\n";
        return;
    }

    const char* sql = "INSERT INTO categorie (nome) VALUES (?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, nome.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            cout << "Categoria inserita correttamente.\n";
        } else {
            cout << "Errore inserimento categoria.\n";
        }
    }
    sqlite3_finalize(stmt);
}

// Modulo 2: registrazione di una nuova spesa
void inserisciSpesa(sqlite3* db) {
    string data, nomeCat, descrizione;
    double importo = 0.0;

    cout << "\n--- Inserisci Spesa ---\n";
    cout << "Data (YYYY-MM-DD): ";
    getline(cin, data);
    data = pulisciSpazi(data);

    cout << "Importo: ";
    if (!(cin >> importo)) {
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Errore: importo non valido.\n";
        return;
    }
    cin.ignore(10000, '\n');

    cout << "Nome categoria: ";
    getline(cin, nomeCat);
    nomeCat = pulisciSpazi(nomeCat);

    cout << "Descrizione (facoltativa): ";
    getline(cin, descrizione);
    descrizione = pulisciSpazi(descrizione);

    if (!dataValida(data)) {
        cout << "Errore: data non valida (usa il formato YYYY-MM-DD).\n";
        return;
    }

    if (importo <= 0) {
        cout << "Errore: l'importo deve essere maggiore di zero.\n";
        return;
    }

    int idCat = cercaCategoria(db, nomeCat);
    if (idCat == -1) {
        cout << "Errore: la categoria non esiste.\n";
        return;
    }

    const char* sql =
        "INSERT INTO spese (data_spesa, importo, id_categoria, descrizione) "
        "VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, data.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 2, importo);
        sqlite3_bind_int(stmt, 3, idCat);

        if (descrizione.empty()) {
            sqlite3_bind_null(stmt, 4);
        } else {
            sqlite3_bind_text(stmt, 4, descrizione.c_str(), -1, SQLITE_TRANSIENT);
        }

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            cout << "Spesa inserita correttamente.\n";
        } else {
            cout << "Errore durante il salvataggio della spesa.\n";
        }
    }
    sqlite3_finalize(stmt);
}

// Modulo 3: impostazione o aggiornamento del budget mensile
void definisciBudget(sqlite3* db) {
    string mese, nomeCat;
    double budget = 0.0;

    cout << "\n--- Definisci Budget Mensile ---\n";
    cout << "Mese (YYYY-MM): ";
    getline(cin, mese);
    mese = pulisciSpazi(mese);

    cout << "Nome categoria: ";
    getline(cin, nomeCat);
    nomeCat = pulisciSpazi(nomeCat);

    cout << "Importo budget: ";
    if (!(cin >> budget)) {
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Errore: valore budget non valido.\n";
        return;
    }
    cin.ignore(10000, '\n');

    if (!meseValido(mese)) {
        cout << "Errore: formato mese non valido (usa YYYY-MM).\n";
        return;
    }

    if (budget <= 0) {
        cout << "Errore: il budget deve essere maggiore di zero.\n";
        return;
    }

    int idCat = cercaCategoria(db, nomeCat);
    if (idCat == -1) {
        cout << "Errore: la categoria non esiste.\n";
        return;
    }

    // Controlla se esiste gia' un budget per quel mese e categoria
    int idBudgetEsistente = -1;
    const char* sqlCheck = "SELECT id_budget FROM budget_mensili WHERE mese = ? AND id_categoria = ?;";
    sqlite3_stmt* stmtCheck = nullptr;

    if (sqlite3_prepare_v2(db, sqlCheck, -1, &stmtCheck, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmtCheck, 1, mese.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmtCheck, 2, idCat);
        if (sqlite3_step(stmtCheck) == SQLITE_ROW) {
            idBudgetEsistente = sqlite3_column_int(stmtCheck, 0);
        }
    }
    sqlite3_finalize(stmtCheck);

    sqlite3_stmt* stmt = nullptr;
    bool ok = false;

    if (idBudgetEsistente != -1) {
        const char* sqlUpdate = "UPDATE budget_mensili SET importo_budget = ? WHERE id_budget = ?;";
        if (sqlite3_prepare_v2(db, sqlUpdate, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_double(stmt, 1, budget);
            sqlite3_bind_int(stmt, 2, idBudgetEsistente);
            ok = (sqlite3_step(stmt) == SQLITE_DONE);
        }
    } else {
        const char* sqlInsert =
            "INSERT INTO budget_mensili (mese, id_categoria, importo_budget) VALUES (?, ?, ?);";
        if (sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, mese.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 2, idCat);
            sqlite3_bind_double(stmt, 3, budget);
            ok = (sqlite3_step(stmt) == SQLITE_DONE);
        }
    }
    sqlite3_finalize(stmt);

    if (ok) {
        cout << "Budget mensile salvato correttamente.\n";
    } else {
        cout << "Errore nel salvataggio del budget.\n";
    }
}

// Report 1: totale speso per ogni categoria
void reportTotaleCategoria(sqlite3* db) {
    cout << "\n--- Totale Spese per Categoria ---\n";
    const char* sql =
        "SELECT c.nome, COALESCE(SUM(s.importo), 0) AS totale "
        "FROM categorie c "
        "LEFT JOIN spese s ON c.id_categoria = s.id_categoria "
        "GROUP BY c.id_categoria, c.nome "
        "ORDER BY totale DESC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        cout << left << setw(20) << "Categoria" << right << setw(15) << "Totale Speso" << "\n";
        cout << string(35, '-') << "\n";

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* cat = (const char*)sqlite3_column_text(stmt, 0);
            double tot = sqlite3_column_double(stmt, 1);
            cout << left << setw(20) << cat
                 << right << setw(15) << fixed << setprecision(2) << tot << "\n";
        }
    }
    sqlite3_finalize(stmt);
}

// Report 2: confronto tra spese mensili e budget impostato
void reportSpeseVsBudget(sqlite3* db) {
    cout << "\n--- Spese Mensili vs Budget ---\n";
    const char* sql =
        "SELECT b.mese, c.nome, b.importo_budget, COALESCE(SUM(s.importo), 0) "
        "FROM budget_mensili b "
        "JOIN categorie c ON b.id_categoria = c.id_categoria "
        "LEFT JOIN spese s ON b.id_categoria = s.id_categoria "
        "                  AND SUBSTR(s.data_spesa, 1, 7) = b.mese "
        "GROUP BY b.id_budget, b.mese, c.nome, b.importo_budget "
        "ORDER BY b.mese DESC, c.nome ASC;";

    sqlite3_stmt* stmt = nullptr;
    bool trovato = false;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            trovato = true;
            const char* mese = (const char*)sqlite3_column_text(stmt, 0);
            const char* cat = (const char*)sqlite3_column_text(stmt, 1);
            double budget = sqlite3_column_double(stmt, 2);
            double speso = sqlite3_column_double(stmt, 3);

            cout << "Mese: " << mese << "\n";
            cout << "Categoria: " << cat << "\n";
            cout << "Budget: " << fixed << setprecision(2) << budget << "\n";
            cout << "Speso: " << fixed << setprecision(2) << speso << "\n";

            if (speso > budget) {
                cout << "Stato: SUPERAMENTO BUDGET\n";
            } else {
                cout << "Stato: ENTRO IL BUDGET\n";
            }
            cout << "--------------------------\n";
        }
    }
    sqlite3_finalize(stmt);

    if (!trovato) {
        cout << "Nessun budget impostato.\n";
    }
}

// Report 3: elenco cronologico completo delle spese
void reportElencoSpese(sqlite3* db) {
    cout << "\n--- Elenco Completo delle Spese ---\n";
    const char* sql =
        "SELECT s.data_spesa, c.nome, s.importo, COALESCE(s.descrizione, '-') "
        "FROM spese s "
        "JOIN categorie c ON s.id_categoria = c.id_categoria "
        "ORDER BY s.data_spesa ASC, s.id_spesa ASC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        cout << left << setw(13) << "Data"
             << setw(16) << "Categoria"
             << right << setw(10) << "Importo" << "  "
             << left << "Descrizione" << "\n";
        cout << string(60, '-') << "\n";

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* data = (const char*)sqlite3_column_text(stmt, 0);
            const char* cat = (const char*)sqlite3_column_text(stmt, 1);
            double imp = sqlite3_column_double(stmt, 2);
            const char* desc = (const char*)sqlite3_column_text(stmt, 3);

            cout << left << setw(13) << data
                 << setw(16) << cat
                 << right << setw(10) << fixed << setprecision(2) << imp << "  "
                 << left << desc << "\n";
        }
    }
    sqlite3_finalize(stmt);
}

// Modulo 4: sottomenu report
void visualizzaReport(sqlite3* db) {
    int scelta = 0;

    do {
        cout << "\nMENU DEI REPORT\n";
        cout << "1. Totale spese per categoria\n";
        cout << "2. Spese mensili vs budget\n";
        cout << "3. Elenco completo delle spese ordinate per data\n";
        cout << "4. Ritorna al menu principale\n";
        cout << "Inserisci la tua scelta: ";

        if (!(cin >> scelta)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Scelta non valida. Riprovare.\n";
            continue;
        }
        cin.ignore(10000, '\n');

        switch (scelta) {
            case 1:
                reportTotaleCategoria(db);
                break;
            case 2:
                reportSpeseVsBudget(db);
                break;
            case 3:
                reportElencoSpese(db);
                break;
            case 4:
                break;
            default:
                cout << "Scelta non valida. Riprovare.\n";
                break;
        }
    } while (scelta != 4);
}

int main() {
    sqlite3* db = nullptr;

    if (sqlite3_open(NOME_DB.c_str(), &db) != SQLITE_OK) {
        cerr << "Impossibile aprire il database: " << sqlite3_errmsg(db) << endl;
        return 1;
    }

    if (!preparaDatabase(db)) {
        sqlite3_close(db);
        return 1;
    }

    cout << "Benvenuto nel Sistema di Gestione delle Spese Personali e del Budget!\n";

    int scelta = 0;
    do {
        cout << "\nSISTEMA SPESE PERSONALI\n";
        cout << "1. Gestione Categorie\n";
        cout << "2. Inserisci Spesa\n";
        cout << "3. Definisci Budget Mensile\n";
        cout << "4. Visualizza Report\n";
        cout << "5. Esci\n";
        cout << "Inserisci la tua scelta: ";

        if (!(cin >> scelta)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Scelta non valida. Riprovare.\n";
            continue;
        }
        cin.ignore(10000, '\n');

        switch (scelta) {
            case 1:
                gestioneCategorie(db);
                break;
            case 2:
                inserisciSpesa(db);
                break;
            case 3:
                definisciBudget(db);
                break;
            case 4:
                visualizzaReport(db);
                break;
            case 5:
                cout << "Uscita dal programma.\n";
                break;
            default:
                cout << "Scelta non valida. Riprovare.\n";
                break;
        }
    } while (scelta != 5);

    sqlite3_close(db);
    return 0;
}