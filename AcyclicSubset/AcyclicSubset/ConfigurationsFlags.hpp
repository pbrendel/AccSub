/* 
 * File:   ConfigurationsFlags.hpp
 * Author: Piotr Brendel
 */

#ifndef CONFIGURATIONSFLAGS_HPP
#define	CONFIGURATIONSFLAGS_HPP

#include <map>

template <typename SimplexType, typename FlagsType>
class ConfigurationsFlags
{
    std::map<SimplexType, FlagsType> flags;

public:

    ConfigurationsFlags()
    {
        // do nothing
    }

    ConfigurationsFlags(int dim, bool subconfigurations, bool addMaximalSimplex)
    {
        Create(dim, subconfigurations, addMaximalSimplex);
    }

    void Create(int dim, bool subconfigurations, bool addMaximalSimplex)
    {
        // uwaga: ponizszy kod jest identyczny z tym w klasie IncidenceGraph
        // do tworzenia flag wszystkich podkonfiguracji
        // generowanie hasha konfiguracja->flaga
        // najpierw tworzymy "bazowy" sympleks maksymalne wymiarowy
        SimplexType s;
        for (int i = 0; i <= dim; i++)
        {
            s.push_back(i);
        }
        // potem generujemy wszystkie sciany posortowane rosnaco wymiarami
        // w kolejnosci leksykograficznej
        std::vector<SimplexType> faces;
        s.GenerateProperFaces(faces);
        if (addMaximalSimplex)
        {
            faces.push_back(s);
        }
        // i tworzymy hasha z flagami konfiguracji
        FlagsType flag = 1;
        for (typename std::vector<SimplexType>::iterator i = faces.begin(); i != faces.end(); i++)
        {
            // tylko flagi dla aktualnego sympleksu, bez podscian
            if (!subconfigurations)
            {
                flags[*i] = flag;
            }
            // dolaczamy flagi podscian
            else
            {
                FlagsType subFlags = flag;
                // teraz generujemy wszystkie podkonfiguracje i aktualizujemy flagi
                // mozemy to zrobic w tym miejscu, bo flagi nizej wymiarowych konfiguracji
                // sa juz ustawione
                std::vector<SimplexType> subconfigurations;
                i->GenerateProperFaces(subconfigurations);
                for (typename std::vector<SimplexType>::iterator j = subconfigurations.begin(); j != subconfigurations.end(); j++)
                {
                    subFlags |= flags[(*j)];
                }
                flags[*i] = subFlags;
            }            
            flag = flag << 1;
        }
    }

    FlagsType operator[] (const SimplexType &s)
    {
        return flags[s];
    }

    bool GetSimplex(FlagsType f, SimplexType &simplex)
    {
        for (typename std::map<SimplexType, FlagsType>::iterator i = flags.begin(); i != flags.end(); i++)
        {
            if (i->second == f)
            {
                simplex = i->first;
                return true;
            }
        }
        return false;
    }
};

#endif	/* CONFIGURATIONSFLAGS_HPP */

