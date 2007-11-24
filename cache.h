//! This file implements a LRU cache template class.

#if ! defined (VPVIEWER_NAVIDBSERVER_CACHE_H)
#define VPVIEWER_NAVIDBSERVER_CACHE_H

#include <map>
#include <utility>


//! LRU cache.
template <typename CacheKey, typename CacheValue>
class Cache
{
public:
    typedef CacheKey key_type;
    typedef CacheValue value_type;

    //! \param cap Capacity of the cache.
    Cache (size_t cap = 3)
        : last_age (0), capacity (cap), hits (0), misses (0)
    { }

    bool get (CacheKey const &, CacheValue *) const;
    void insert (CacheKey const &, CacheValue const &);
    void clear ();
    void set_capacity (size_t);

private:
    void trim_to_capacity ();

    //! The type of cache's/items' age.
    typedef unsigned long age_type;

    typedef std::map<CacheKey,
        std::pair<CacheValue, age_type> > cache_type;
    //! \c CacheKey -> (\c CacheValue, age).
    mutable cache_type cache;

    //! \c Age -> \c CacheKey mapping.
    mutable std::map<age_type, CacheKey> ages;

    //! Last age.
    mutable age_type last_age;

    //! Capacity of the cache.
    size_t capacity;

    //! Statistics counters.
    mutable unsigned long hits, misses;
};


//! Retrieve a value from cache.
// \param key Key.
// \param value Pointer to variable that we store the value.
// \returns True if the key is in the cache, otherwise false.
template <typename CacheKey, typename CacheValue>
bool
Cache<CacheKey, CacheValue>::get (CacheKey const & key, 
                                  CacheValue * value) const
{
    //CacheValue value;
    cache_type::iterator it = cache.find (key);
    if (it != cache.end ())
    {
        *value = it->second.first;

        age_type new_last_age = last_age + 1;
        if (new_last_age < last_age)
        {
            cache.clear ();
            ages.clear ();
        }
        last_age = new_last_age;

        // Update the age -> mesh ID mapping.
        ages.erase (it->second.second);
        ages.insert (ages.end (), std::make_pair (new_last_age, key));
        // Make the found item younger.
        it->second.second = new_last_age;

        hits += 1;
        return true;
    }
    else
    {
        misses += 1;
        return false;
    }
}


//! Inserts an element into the cache.
// \param key Key.
// \param value Value associated with the key.
template <typename CacheKey, typename CacheValue>
void
Cache<CacheKey, CacheValue>::insert (CacheKey const & key, 
                                     CacheValue const & value)
{
    age_type new_last_age = last_age + 1;
    if (new_last_age < last_age)
    {
        cache.clear ();
        ages.clear ();
    }
    last_age = new_last_age;

    cache_type::iterator it = cache.find (key);
    // Insert new element if it does not exist in cache.
    if (it == cache.end ())
    {
        cache.insert (std::make_pair (key,
            std::make_pair (value, new_last_age)));
        ages.insert (ages.end (), std::make_pair (new_last_age, key));
    }
    // Or update the record of existing element.
    else
    {
        ages.erase (it->second.second);
        ages.insert (ages.end (), std::make_pair (new_last_age, key));
        it->second.first = value;
        it->second.second = new_last_age;
    }

    trim_to_capacity ();
}


//! Empties the cache.
template <typename CacheKey, typename CacheValue>
void
Cache<CacheKey, CacheValue>::clear ()
{
    cache.clear ();
    ages.clear ();
    last_age = 0;
}


//! Sets capacity of the cache and also trims the cache if the \c new_cap is
// smaller than previous value.
// \param new_cap New capacity of the cache.
template <typename CacheKey, typename CacheValue>
void
Cache<CacheKey, CacheValue>::set_capacity (size_t new_cap)
{
    capacity = new_cap;
    trim_to_capacity ();
}


//! Removes the oldest items in cache if the number of items in the cace
// is bigger than its capacity.
template <typename CacheKey, typename CacheValue>
void
Cache<CacheKey, CacheValue>::trim_to_capacity ()
{
    while (cache.size () > capacity)
    {
        std::map<age_type, CacheKey>::iterator ages_it
            = ages.begin ();
        cache.erase (ages_it->second);
        ages.erase (ages_it);
    }
}


#endif // VPVIEWER_NAVIDBSERVER_CACHE_H
