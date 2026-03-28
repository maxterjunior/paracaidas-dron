# Paracaídas para Drones — Sistema de Despliegue de Emergencia

Sistema de paracaídas de emergencia que detecta fallos en el dron y despliega automáticamente el paracaídas.

## Estructura del Proyecto

```
paracaidas-dron/
├── docs/
│   ├── investigacion/      # Research de módulos y componentes
│   └── esquematicos/       # Diagramas y esquemáticos de circuito
├── firmware/               # Código embebido del microcontrolador
│   ├── src/
│   └── lib/
├── hardware/               # Archivos de diseño PCB (KiCad / EasyEDA)
└── README.md
```

## Áreas del Proyecto

1. **Investigación de módulos** — sensores IMU, acelerómetros, actuadores de despliegue, microcontroladores
2. **Esquemáticos** — diseño del circuito de detección y disparo
3. **Firmware** — lógica de detección de caída libre y control del mecanismo de despliegue
