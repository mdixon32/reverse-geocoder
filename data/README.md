# Country Data

This folder is where compiled country boundary data lives.

Recommended approach:

- run the importer in `tools/import_country.py`
- let it create one `.cpp` file per country in `data/countries/`
- let it refresh the generated registry in `data/generated/`
- keep coastlines simplified before embedding them

Points must be stored as:

```cpp
rg::Coordinate{longitude, latitude}
```

Not:

```cpp
rg::Coordinate{latitude, longitude}
```
