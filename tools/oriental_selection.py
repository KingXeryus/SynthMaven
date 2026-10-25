"""FM1 bank selection: melodic recipes, Dry and Expressive, without send FX."""
def select_presets(reference):
    assert len(reference) == 208
    selected = []
    for recipe in range(44):
        for variation in (0, 2):
            source = reference[recipe * 4 + variation]
            item = dict(source, values=list(source['values']))
            for parameter in (34, 35, 36):
                item['values'][parameter] = 0
            selected.append(item)
    return selected
