-- Script SQL per il Sistema di Gestione delle Spese Personali e del Budget
-- File: sql/database.sql

PRAGMA foreign_keys = ON;

DROP TABLE IF EXISTS spese;
DROP TABLE IF EXISTS budget_mensili;
DROP TABLE IF EXISTS categorie;

-- Creazione tabelle e vincoli di integrita'

CREATE TABLE categorie (
    id_categoria INTEGER PRIMARY KEY AUTOINCREMENT,
    nome VARCHAR(100) NOT NULL UNIQUE,
    CHECK (LENGTH(TRIM(nome)) > 0)
);

CREATE TABLE spese (
    id_spesa INTEGER PRIMARY KEY AUTOINCREMENT,
    data_spesa DATE NOT NULL,
    importo DECIMAL(10, 2) NOT NULL,
    id_categoria INTEGER NOT NULL,
    descrizione TEXT,
    CHECK (LENGTH(data_spesa) = 10),
    CHECK (importo > 0),
    FOREIGN KEY (id_categoria) REFERENCES categorie(id_categoria)
        ON DELETE RESTRICT
        ON UPDATE CASCADE
);

CREATE TABLE budget_mensili (
    id_budget INTEGER PRIMARY KEY AUTOINCREMENT,
    mese CHAR(7) NOT NULL,
    id_categoria INTEGER NOT NULL,
    importo_budget DECIMAL(10, 2) NOT NULL,
    CHECK (LENGTH(mese) = 7),
    CHECK (importo_budget > 0),
    UNIQUE (mese, id_categoria),
    FOREIGN KEY (id_categoria) REFERENCES categorie(id_categoria)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Dati di esempio

INSERT INTO categorie (nome) VALUES ('Alimentari');
INSERT INTO categorie (nome) VALUES ('Trasporti');
INSERT INTO categorie (nome) VALUES ('Utenze');
INSERT INTO categorie (nome) VALUES ('Svago');
INSERT INTO categorie (nome) VALUES ('Salute');

INSERT INTO budget_mensili (mese, id_categoria, importo_budget) VALUES ('2025-01', 1, 300.00);
INSERT INTO budget_mensili (mese, id_categoria, importo_budget) VALUES ('2025-01', 2, 150.00);
INSERT INTO budget_mensili (mese, id_categoria, importo_budget) VALUES ('2025-01', 3, 200.00);
INSERT INTO budget_mensili (mese, id_categoria, importo_budget) VALUES ('2025-01', 4, 100.00);

INSERT INTO spese (data_spesa, importo, id_categoria, descrizione)
VALUES ('2025-01-05', 145.50, 1, 'Spesa settimanale supermercato');

INSERT INTO spese (data_spesa, importo, id_categoria, descrizione)
VALUES ('2025-01-10', 55.00, 2, 'Abbonamento autobus');

INSERT INTO spese (data_spesa, importo, id_categoria, descrizione)
VALUES ('2025-01-12', 85.00, 3, 'Bolletta luce');

INSERT INTO spese (data_spesa, importo, id_categoria, descrizione)
VALUES ('2025-01-15', 25.00, 1, 'Pranzo');

INSERT INTO spese (data_spesa, importo, id_categoria, descrizione)
VALUES ('2025-01-18', 65.00, 2, 'Rifornimento benzina');

INSERT INTO spese (data_spesa, importo, id_categoria, descrizione)
VALUES ('2025-01-22', 150.00, 1, 'Spesa alimentari e dispensa');

INSERT INTO spese (data_spesa, importo, id_categoria, descrizione)
VALUES ('2025-01-25', 40.00, 4, 'Cinema e pizza');

INSERT INTO spese (data_spesa, importo, id_categoria, descrizione)
VALUES ('2025-01-28', 32.00, 5, 'Farmacia');