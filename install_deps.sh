#!/bin/bash

# Script para instalar dependencias en el entorno correcto

echo "🔧 Instalando NetworkX..."

# Intentar con pip3
if command -v pip3 &> /dev/null; then
    pip3 install --user networkx
    echo "✅ NetworkX instalado con pip3 --user"
elif command -v python3 &> /dev/null; then
    python3 -m pip install --user networkx
    echo "✅ NetworkX instalado con python3 -m pip --user"
else
    echo "⚠️  No se pudo instalar automáticamente."
    echo "Por favor ejecuta: pip3 install --user networkx"
fi
