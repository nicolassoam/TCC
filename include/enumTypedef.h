#ifndef ENUMTYPEDEF_H
#define ENUMTYPEDEF_H

enum Sheets
{
    CASK = 0,
    PASSAGEM = 1,
    ROTAS = 2,
    ROTAS2 = 3,
    ROTAS3 = 4
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




#endif