#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include "cache.hpp"


double
xrandom ()
{
  return std::rand () / (RAND_MAX + 1.0);
}


unsigned
xrandom (unsigned mod)
{
  return unsigned(double(std::rand ()) / (RAND_MAX + 1.0) * mod);
}


int value;


int
main ()
{
    std::srand (static_cast<unsigned>(std::time (0) ^ getpid ()));

    Cache<int, int> int_cache (10000);    
    for (size_t i = 0; i != int_cache.get_capacity (); ++i)
    {
	int_cache.insert (xrandom (1000000), xrandom (1000000));
    }

    Cache<int, int> const & cache_ref = int_cache;
    for (size_t i = 0; i != cache_ref.get_capacity () * 100; ++i)
    {
	cache_ref.get (xrandom (1000000), &value);
    }

    std::cout << "last value: " << value << std::endl;

    std::pair<unsigned long, unsigned long> stats = cache_ref.get_stats ();
    std::cout << "hits: " << stats.first << " / misses: " << stats.second
	      << std::endl;
}
