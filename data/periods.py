"""
Configuration for Academic Periods
"""

PERIODOS = {
    "sept-dec": {  # Septiembre a Diciembre (1 Sep - 12 Dic = 15 semanas)
        "semanas": 15,
        "cuatrimestres": {
            1: [101, 102, 103, 104, 105, 106, 107],   # Materias del cuatri 1
            4: [401, 402, 403, 404, 405, 406, 407],   # Materias del cuatri 4
            7: [701, 702, 703, 704, 705, 706, 707],   # Materias del cuatri 7
            10: [1001]  # Estancias
        }
    },
    
    "jan-apr": {  # Enero a Abril (5 Ene - 24 Abr = 15 semanas)
        "semanas": 15,
        "cuatrimestres": {
            2: [201, 202, 203, 204, 205, 206, 207],   # Materias del cuatri 2
            5: [501, 502, 503, 504, 505, 506, 507],   # Materias del cuatri 5
            8: [801, 802, 803, 804, 805, 806, 807]    # Materias del cuatri 8
        }
    },
    
    "may-aug": {  # Mayo a Agosto (4 May - 21 Ago = 15 semanas)
        "semanas": 15,
        "cuatrimestres": {
            3: [301, 302, 303, 304, 305, 306, 307],   # Materias del cuatri 3
            6: [601],  # Estancias
            9: [901, 902, 903, 904, 905, 906, 907]    # Materias del cuatri 9
        }
    }
}

def get_valid_semesters(period_key):
    """Get list of valid semester numbers for a given period key"""
    if period_key not in PERIODOS:
        return []
    return list(PERIODOS[period_key]["cuatrimestres"].keys())

def get_valid_groups(period_key):
    """Get list of valid group IDs for a given period key"""
    if period_key not in PERIODOS:
        return []
    
    groups = []
    for sem_groups in PERIODOS[period_key]["cuatrimestres"].values():
        groups.extend(sem_groups)
    return groups
