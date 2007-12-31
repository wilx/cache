//! This file implements a LRU cache template class.

#if ! defined (LRU_CACHE_H)
#define LRU_CACHE_H

#include <map>
#include <utility>
#include <boost/call_traits.hpp>
#include <memory>
#include <functional>


//! LRU cache.
template <typename CacheKey, typename CacheValue, 
    template <typename T> class Allocator = std::allocator >
class Cache
{
public:
    typedef CacheKey key_type;
    typedef CacheValue value_type;

    typedef typename boost::call_traits<key_type>::param_type
        key_param_type;
    typedef typename boost::call_traits<value_type>::param_type
        value_param_type;

    //! \param cap Capacity of the cache.
    Cache (size_t cap = 3)
        : last_age (0), capacity (cap), hits (0), misses (0)
    { }

    bool get (key_param_type, value_type *) const;
    void insert (key_param_type, value_param_type);
    void clear ();
    void set_capacity (size_t);
    size_t get_capacity () const;

    std::pair<unsigned long, unsigned long> get_stats () const;

private:
    void trim_to_capacity ();

    //! The type of cache's/items' age.
    typedef unsigned long age_type;

    //! Allocator for \c cache associative container.
    typedef Allocator<std::pair<const key_type, std::pair<value_type, age_type>
			  > > cache_allocator_type;

    //! The type of associative container for 
    // \c CacheKey -> (\c CacheValue, age) map.
    typedef std::map<key_type, std::pair<value_type, age_type>, 
	std::less<key_type>,  cache_allocator_type> cache_type;

    //! \c CacheKey -> (\c CacheValue, age).
    mutable cache_type cache;

    //! Allocator for the \c ages associative container.
    typedef Allocator<std::pair<const age_type, key_type> > ages_allocator_type;

    //! \c Age -> \c CacheKey mapping.
    mutable std::map<age_type, key_type, std::less<age_type>,
	ages_allocator_type> ages;

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
template <typename CacheKey, typename CacheValue, 
    template <typename> class Allocator>
inline
bool
Cache<CacheKey, CacheValue, Allocator>::get (key_param_type key, 
    value_type * value) const
{
    typename cache_type::iterator it = cache.find (key);
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
template <typename CacheKey, typename CacheValue,
    template <typename> class Allocator>
inline
void
Cache<CacheKey, CacheValue, Allocator>::insert (key_param_type key, 
    value_param_type value)
{
    age_type new_last_age = last_age + 1;
    if (new_last_age < last_age)
    {
        cache.clear ();
        ages.clear ();
    }
    last_age = new_last_age;

    typename cache_type::iterator it = cache.find (key);
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
template <typename CacheKey, typename CacheValue, 
    template <typename> class Allocator>
inline
void
Cache<CacheKey, CacheValue, Allocator>::clear ()
{
    cache.clear ();
    ages.clear ();
    last_age = 0;
}


//! Sets capacity of the cache and also trims the cache if the \c new_cap is
// smaller than previous value.
// \param new_cap New capacity of the cache.
template <typename CacheKey, typename CacheValue,
    template <typename> class Allocator>
inline
void
Cache<CacheKey, CacheValue, Allocator>::set_capacity (size_t new_cap)
{
    capacity = new_cap;
    trim_to_capacity ();
}


//! Returns maximum size of the cache.
template <typename CacheKey, typename CacheValue,
    template <typename> class Allocator>
inline
size_t 
Cache<CacheKey, CacheValue, Allocator>::get_capacity () const
{
    return capacity;
}


//! Removes the oldest items in cache if the number of items in the cace
// is bigger than its capacity.
template <typename CacheKey, typename CacheValue,
    template <typename> class Allocator>
inline
void
Cache<CacheKey, CacheValue, Allocator>::trim_to_capacity ()
{
    while (cache.size () > capacity)
    {
        typename std::map<age_type, key_type>::iterator ages_it
            = ages.begin ();
        cache.erase (ages_it->second);
        ages.erase (ages_it);
    }
}


//! Returns hits and misses statistics. Hits are first, misses are second.
template <typename CacheKey, typename CacheValue,
    template <typename> class Allocator>
inline
std::pair<unsigned long, unsigned long>
Cache<CacheKey, CacheValue, Allocator>::get_stats () const
{
    return std::make_pair (hits, misses);
}


#endif // LRU_CACHE_H
