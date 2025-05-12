#ifndef ENUMTYPEDEF_H
#define ENUMTYPEDEF_H

enum Sheets
{
    AERONAVE, 
    CASK,
    PASSAGEM,
    ROTAS,
    ROTAS2
};

namespace Rotas
{
    enum CollumnsRotas
    {
        OD,
        ORIGEM,
        DESTINO,
        PISTA,
        DEMANDA,
        DISTANCIA
    };
}

namespace Rotas2_3
{ 
    enum CollumnsRotas2_3
    {
        OD, 
        ORIGEM, 
        DESTINO, 
        PISTA, 
        TURNO,
        DEMANDA, 
        DISTANCIA
    };
}

namespace CaskPassagem
{
    enum CollumnsCaskPassagem
    {
        OD,
        DESTINO,
        E190_E2,
        E195_E2, 
        A220_100, 
        A220_300,
        A319Neo,
        B737_M7,
        ATR_72
    };
}

namespace AeronaveCols
{
    enum CollumnsAeronave
    {
        E190_E2,
        E195_E2, 
        A220_100, 
        A220_300,
        A319Neo,
        B737_M7,
        ATR_72  
    };
};

enum Aeronave
{
    FROTA,
    ASSENTOS,
    ALCANCE,
    PISTA
};


#endif