#!/usr/bin/env python3
"""
Genera esquemático KiCad para sistema paracaídas DJI Matrice 400.
Posiciones calculadas con la misma fórmula que el analizador kicad-happy:
  abs_x = cx + rpx
  abs_y = cy - rpy   (Y INVERTIDO — símbolos usan math-Y-up, esquemático usa screen-Y-down)
"""
import uuid, math

def u(): return str(uuid.uuid4())
def esc(s): return f'"{s}"'

# ── Pin position (same formula as kicad-happy analyzer) ──────────────────────
def apply_rotation(px, py, angle_deg):
    if angle_deg == 0:
        return px, py
    rad = math.radians(angle_deg)
    c = round(math.cos(rad), 10)
    s = round(math.sin(rad), 10)
    return (px*c - py*s, px*s + py*c)

def pin_pos(cx, cy, angle, px, py):
    """Absolute sheet position of a pin — matches kicad-happy compute_pin_positions."""
    rpx, rpy = apply_rotation(px, py, angle)
    return round(cx + rpx, 4), round(cy - rpy, 4)   # Y INVERTED

# ── Symbol pin positions (in symbol math-coordinates, Y-up) ──────────────────
# These match the pin definitions in lib_symbols below.

# Resistor: pin1=(0,3.81) top, pin2=(0,-3.81) bottom
R = {'1':(0,3.81), '2':(0,-3.81)}

# Capacitor: pin1=(0,2.54), pin2=(0,-2.54)
C = {'1':(0,2.54), '2':(0,-2.54)}

# Inductor: pin1=(0,5.08) top, pin2=(0,-5.08) bottom
L = {'1':(0,5.08), '2':(0,-5.08)}

# LED: A=(-3.81,0) left, K=(3.81,0) right
LED = {'A':(-3.81,0), 'K':(3.81,0)}

# Diode: A=(-3.81,0) left, K=(3.81,0) right
D = {'A':(-3.81,0), 'K':(3.81,0)}

# NPN transistor: B=(0,0), C=(5.08,2.54), E=(5.08,-2.54)
NPN = {'B':(0,0), 'C':(5.08,2.54), 'E':(5.08,-2.54)}

# NMOS transistor: G=(0,0), D=(5.08,2.54), S=(5.08,-2.54)
NMOS = {'G':(0,0), 'D':(5.08,2.54), 'S':(5.08,-2.54)}

# Battery: +=(0,2.54), -=(0,-2.54)
BAT = {'+': (0,2.54), '-': (0,-2.54)}

# SW_SPST: 1=(-3.81,0), 2=(3.81,0)
SW_SPST = {'1':(-3.81,0), '2':(3.81,0)}

# SW_Push: 1=(-2.032,0), 2=(2.032,0)
SW_PUSH = {'1':(-2.032,0), '2':(2.032,0)}

# Buzzer: K=(0,2.286) top, +=(−3.81,0) left  [pin length=0 so connection IS at these coords]
BUZ = {'K':(0,2.286), '+'  :(-3.81,0)}

# Solenoid: 1=(0,3.81) top, 2=(0,-3.81) bottom  [pin length=1.27 so connection at stated pos]
SOL = {'1':(0,3.81), '2':(0,-3.81)}

# MCP1700: IN=(-5.08,1.27), GND=(-5.08,-1.27), OUT=(5.08,0)
MCP = {'IN':(-5.08,1.27), 'GND':(-5.08,-1.27), 'OUT':(5.08,0)}

# MT3608: IN=(-5.08,3.81), GND=(-5.08,1.27), FB=(-5.08,-1.27), EN=(-5.08,-3.81), SW=(5.08,3.81), BS=(5.08,-3.81)
MT = {'IN':(-5.08,3.81), 'GND':(-5.08,1.27), 'FB':(-5.08,-1.27), 'EN':(-5.08,-3.81),
      'SW':(5.08,3.81), 'BS':(5.08,-3.81)}

# PIC18LF25K22: 28-DIP
# Left pins (1-14): x=-7.62, py from 16.51 down to -16.51 (step -2.54)
# Right pins (15-28): x=7.62, py from -16.51 up to 16.51 (step +2.54)
PIC = {}
left_pins = ['MCLR','RA0','RA1','RA2','RA3','RA4','RA5','VSS',
             'RA7','RA6','RC0','RC1','RC2','RC3']
right_pins = ['RC4','RC5','RC6','RC7','VSS2','VDD','RB0','RB1',
              'RB2','RB3','RB4','RB5','RB6','RB7']
for i,name in enumerate(left_pins):
    py = 16.51 - i*2.54
    PIC[name] = (-7.62, py)
    PIC[str(i+1)] = (-7.62, py)
for i,name in enumerate(right_pins):
    py = -16.51 + i*2.54
    PIC[name] = (7.62, py)
    PIC[str(i+15)] = (7.62, py)

# MPU-6050: left pins at x=-7.62, right pins at x=7.62
MPU = {
    'VCC':(-7.62,5.08), 'GND':(-7.62,2.54), 'SCL':(-7.62,0.0), 'SDA':(-7.62,-2.54),
    'INT':(7.62,5.08), 'AD0':(7.62,2.54), 'XCL':(7.62,0.0), 'XDA':(7.62,-2.54),
}

# Connector 1x04: pins at (0, 3.81), (0, 1.27), (0, -1.27), (0, -3.81)
CONN4 = {'1':(0,3.81), '2':(0,1.27), '3':(0,-1.27), '4':(0,-3.81)}

# Footprint shortcuts
FP_R    = 'Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal'
FP_C    = 'Capacitor_THT:C_Disc_D5.0mm_W2.5mm_P5.00mm'
FP_CP   = 'Capacitor_THT:CP_Radial_D6.3mm_P2.50mm'
FP_LED  = 'LED_THT:LED_D5.0mm'
FP_D    = 'Diode_THT:D_DO-41_SOD81_P10.16mm_Horizontal'
FP_L    = 'Inductor_THT:L_Axial_L5.3mm_D2.2mm_P10.16mm_Horizontal'
FP_NPN  = 'Package_TO_SOT_THT:TO-92_Inline'
FP_NMOS = 'Package_TO_SOT_THT:TO-220-3_Vertical'
FP_CONN = 'Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical'
FP_SW   = 'Button_Switch_THT:SW_Slide_1P2T_CK_OS102011MS2Q'
FP_PUSH = 'Button_Switch_THT:SW_PUSH_6mm'

# ── Component placements: (lib_id, ref, value, cx, cy, angle, pin_map, footprint) ──
COMPS = [
    # Power supply block (left side)
    ('Device:Battery_Cell',              'BT1',   'LiPo_1S_1000mAh', -120, 50,  0, BAT,     'Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical'),
    ('Switch:SW_SPST',                   'SW1',   'SW_ON-OFF',        -105, 50,  0, SW_SPST, FP_SW),
    ('Regulator_Linear:MCP1700-3302E',   'U3',    'MCP1700-3302E',     -88, 18,  0, MCP,     'Package_TO_SOT_SMD:SOT-23'),
    ('Device:C',                         'C2',    '10uF',              -72, 18,  0, C,       FP_CP),
    ('Regulator_Switching:MT3608',       'U4',    'MT3608',            -75, 50,  0, MT,      'Package_TO_SOT_SMD:SOT-23-6'),
    ('Device:C',                         'C5',    '100nF',             -90, 62,  0, C,       FP_C),
    ('Device:L',                         'L1',    '22uH_1A',           -55, 38, 90, L,       FP_L),
    ('Device:C',                         'C4',    '100uF',             -40, 50,  0, C,       FP_CP),
    ('Device:R',                         'R_FB1', '750k',              -58, 62,  0, R,       FP_R),
    ('Device:R',                         'R_FB2', '100k',              -58, 72,  0, R,       FP_R),
    # MCU block (center)
    ('MCU:PIC18LF25K22',                 'U1',    'PIC18LF25K22',        0, 20,  0, PIC,     'Package_DIP:DIP-28_W7.62mm'),
    ('Device:R',                         'R1',    '10k',               -28,-10,  0, R,       FP_R),
    ('Switch:SW_Push',                   'SW2',   'SW_Push_MCLR',      -28,  0,  0, SW_PUSH, FP_PUSH),
    ('Device:C',                         'C1',    '100nF',              22, -5,  0, C,       FP_C),
    # IMU block (upper right)
    ('Sensor_Motion:MPU-6050',           'U2',    'MPU-6050',           80,-20,  0, MPU,     'Connector_PinHeader_2.54mm:PinHeader_2x04_P2.54mm_Vertical'),
    ('Device:R',                         'R5',    '4k7',                62,-43,  0, R,       FP_R),
    ('Device:R',                         'R6',    '4k7',                74,-43,  0, R,       FP_R),
    ('Device:C',                         'C3',    '100nF',             100,-32,  0, C,       FP_C),
    # Output block (right)
    ('Device:R',                         'R3',    '330',                55, -5, 90, R,       FP_R),
    ('Device:LED',                       'LED1',  'LED_Verde',          68, -5, 90, LED,     FP_LED),
    ('Device:R',                         'R4',    '330',                55,  5, 90, R,       FP_R),
    ('Device:LED',                       'LED2',  'LED_Rojo',           68,  5, 90, LED,     FP_LED),
    ('Device:R',                         'R2',    '10k',                55, 20,  0, R,       FP_R),
    ('Device:Q_NMOS_GDS',                'Q1',    'IRLZ44N',            72, 20,  0, NMOS,    FP_NMOS),
    ('Device:D',                         'D1',    '1N4007',             88, 12,  0, D,       FP_D),
    ('Mechanical:Solenoid',              'SOL1',  'Solenoid_5V',       105, 10,  0, SOL,     'Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical'),
    ('Device:R',                         'R7',    '1k',                 55, 40, 90, R,       FP_R),
    ('Device:Q_NPN_BCE',                 'Q2',    '2N3904',             72, 40,  0, NPN,     FP_NPN),
    ('Device:Buzzer',                    'BZ1',   'Buzzer_5V',          90, 37,  0, BUZ,     'Buzzer_Beeper:ABT-410-RC'),
    # Connectors
    ('Connector_Generic:Conn_01x04',     'J1',    'UART_Debug',        -10, 75,  0, CONN4,   FP_CONN),
    ('Connector_Generic:Conn_01x04',     'J2',    'ICSP_PICKit',        20, 75,  0, CONN4,   FP_CONN),
]

# ── Net connectivity ──────────────────────────────────────────────────────────
# Each net: list of (ref, pin_name) that share the same net label/power symbol
NETS = {
    'VBAT':     [('BT1','+'), ('SW1','1')],
    'VBAT_SW':  [('SW1','2'), ('U4','IN'), ('U4','EN'), ('U3','IN'), ('C5','1')],
    '+3V3':     [('U3','OUT'), ('C2','1'), ('U1','VDD'), ('C1','1'),
                 ('R5','1'), ('R6','1'), ('U2','VCC'), ('C3','1'), ('R1','1')],
    'GND':      [('BT1','-'), ('U3','GND'), ('C2','2'), ('U4','GND'), ('C5','2'),
                 ('C4','2'), ('R_FB2','2'),
                 ('U1','VSS'), ('U1','VSS2'), ('C1','2'), ('SW2','2'),
                 ('U2','GND'), ('C3','2'), ('U2','AD0'),
                 ('R2','2'), ('Q1','S'), ('Q2','E'),
                 ('LED1','K'), ('LED2','K'),
                 ('J1','1'), ('J2','2')],
    '+5V':      [('U4','BS'), ('L1','2'), ('R_FB1','1'), ('C4','1'),
                 ('SOL1','1'), ('BZ1','+'), ('D1','K'),
                 ('J1','4'), ('J2','1')],
    'BOOST_SW': [('U4','SW'), ('L1','1')],
    'BOOST_FB': [('U4','FB'), ('R_FB1','2'), ('R_FB2','1')],
    'MCLR':     [('U1','MCLR'), ('R1','2'), ('SW2','1')],
    'SCL':      [('U1','RC3'), ('U2','SCL'), ('R5','2')],
    'SDA':      [('U1','RC4'), ('U2','SDA'), ('R6','2')],
    'MPU_INT':  [('U1','RB0'), ('U2','INT')],
    'SOL_CTRL': [('U1','RC2'), ('Q1','G'), ('R2','1')],
    'DRAIN_Q1': [('Q1','D'), ('D1','A'), ('SOL1','2')],
    'BUZ_CTRL': [('U1','RC0'), ('R7','2')],
    'Q2_BASE':  [('R7','1'), ('Q2','B')],
    'BZ_COL':   [('Q2','C'), ('BZ1','K')],
    'LED1_CTRL':[('U1','RA0'), ('R3','2')],
    'LED1_A':   [('R3','1'), ('LED1','A')],
    'LED2_CTRL':[('U1','RA1'), ('R4','2')],
    'LED2_A':   [('R4','1'), ('LED2','A')],
    'UART_TX':  [('U1','RC6'), ('J1','3')],
    'UART_RX':  [('U1','RC7'), ('J1','2')],
    'PGC':      [('U1','RB6'), ('J2','3')],
    'PGD':      [('U1','RB7'), ('J2','4')],
}

# No-connects (unused pins)
NC = [
    ('U1','RA2'),('U1','RA3'),('U1','RA4'),('U1','RA5'),
    ('U1','RA7'),('U1','RA6'),('U1','RC1'),('U1','RC5'),
    ('U1','RB1'),('U1','RB2'),('U1','RB3'),('U1','RB4'),('U1','RB5'),
    ('U2','XCL'),('U2','XDA'),
]

# ── S-expression helpers ──────────────────────────────────────────────────────
def prop(name, val, x=0, y=0, angle=0, size=1.27, hide=False):
    h = ' (hide yes)' if hide else ''
    return (f'(property {esc(name)} {esc(val)} (at {x} {y} {angle})\n'
            f'  (effects (font (size {size} {size})){h}))')

def pdef(num, name, x, y, angle, ptype='passive', length=2.54):
    return (f'(pin {ptype} line (at {x} {y} {angle}) (length {length})\n'
            f'  (name {esc(str(name))} (effects (font (size 1.27 1.27))))\n'
            f'  (number {esc(str(num))} (effects (font (size 1.27 1.27)))))')

def rect(x1,y1,x2,y2):
    return f'(rectangle (start {x1} {y1}) (end {x2} {y2}) (stroke (width 0)(type default))(fill(type none)))'

def poly(*pts):
    return f'(polyline (pts {" ".join(f"(xy {x} {y})" for x,y in pts)}) (stroke (width 0)(type default))(fill(type none)))'

def arc3(sx,sy,mx,my,ex,ey):
    return f'(arc (start {sx} {sy}) (mid {mx} {my}) (end {ex} {ey}) (stroke (width 0)(type default))(fill(type none)))'

# ── Library symbols ───────────────────────────────────────────────────────────
def lib_symbols():
    s = '(lib_symbols\n'

    # Resistor
    s += f'(symbol "Device:R"\n  (pin_numbers(hide yes))(pin_names(offset 0))\n  (exclude_from_sim no)(in_bom yes)(on_board yes)\n'
    s += f'  {prop("Reference","R",1.524,0,90)}\n  {prop("Value","R",-1.524,0,90)}\n  {prop("Footprint","",hide=True)}\n  {prop("Datasheet","~",hide=True)}\n'
    s += f'  (symbol "R_0_1" {rect(-1.016,-2.032,1.016,2.032)})\n'
    s += f'  (symbol "R_1_1"\n    {pdef(1,"~",0,3.81,270)}\n    {pdef(2,"~",0,-3.81,90)})\n)\n'

    # Capacitor (non-polarized)
    s += f'(symbol "Device:C"\n  (pin_numbers(hide yes))(pin_names(offset 0.254))\n  (exclude_from_sim no)(in_bom yes)(on_board yes)\n'
    s += f'  {prop("Reference","C",1.524,0,90)}\n  {prop("Value","C",-1.524,0,90)}\n  {prop("Footprint","",hide=True)}\n  {prop("Datasheet","~",hide=True)}\n'
    s += f'  (symbol "C_0_1" {poly((-2.032,1.016),(2.032,1.016))} {poly((-2.032,-1.016),(2.032,-1.016))})\n'
    s += f'  (symbol "C_1_1"\n    {pdef(1,"+",0,2.54,270)}\n    {pdef(2,"-",0,-2.54,90)})\n)\n'

    # Inductor
    s += f'(symbol "Device:L"\n  (pin_numbers(hide yes))(pin_names(offset 1.016))\n  (exclude_from_sim no)(in_bom yes)(on_board yes)\n'
    s += f'  {prop("Reference","L",1.778,0,90)}\n  {prop("Value","L",-1.778,0,90)}\n  {prop("Footprint","",hide=True)}\n  {prop("Datasheet","~",hide=True)}\n'
    s += f'  (symbol "L_0_1"\n    {arc3(0,-3.81,1.27,-2.54,0,-1.27)}\n    {arc3(0,-1.27,1.27,0,0,1.27)}\n    {arc3(0,1.27,1.27,2.54,0,3.81)})\n'
    s += f'  (symbol "L_1_1"\n    {pdef(1,"~",0,5.08,270)}\n    {pdef(2,"~",0,-5.08,90)})\n)\n'

    # Diode
    s += f'(symbol "Device:D"\n  (pin_numbers(hide yes))(pin_names(offset 0))\n  (exclude_from_sim no)(in_bom yes)(on_board yes)\n'
    s += f'  {prop("Reference","D",0,2.54,0)}\n  {prop("Value","D",0,-2.54,0)}\n  {prop("Footprint","",hide=True)}\n  {prop("Datasheet","~",hide=True)}\n'
    s += f'  (symbol "D_0_1" {poly((-1.27,1.27),(1.27,0),(-1.27,-1.27),(-1.27,1.27))} {poly((1.27,1.27),(1.27,-1.27))})\n'
    s += f'  (symbol "D_1_1"\n    {pdef("A","A",-3.81,0,0)}\n    {pdef("K","K",3.81,0,180)})\n)\n'

    # LED
    s += f'(symbol "Device:LED"\n  (pin_numbers(hide yes))(pin_names(offset 0))\n  (exclude_from_sim no)(in_bom yes)(on_board yes)\n'
    s += f'  {prop("Reference","D",0,2.54,0)}\n  {prop("Value","LED",0,-2.54,0)}\n  {prop("Footprint","",hide=True)}\n  {prop("Datasheet","~",hide=True)}\n'
    s += f'  (symbol "LED_0_1" {poly((-1.27,1.27),(1.27,0),(-1.27,-1.27),(-1.27,1.27))} {poly((1.27,1.27),(1.27,-1.27))} {poly((0.381,1.905),(1.143,2.667),(1.651,2.159))} {poly((1.143,1.397),(1.905,2.159),(2.413,1.651))})\n'
    s += f'  (symbol "LED_1_1"\n    {pdef("A","A",-3.81,0,0)}\n    {pdef("K","K",3.81,0,180)})\n)\n'

    # NPN
    s += f'(symbol "Device:Q_NPN_BCE"\n  (pin_names(offset 1.016))\n  (exclude_from_sim no)(in_bom yes)(on_board yes)\n'
    s += f'  {prop("Reference","Q",5.08,5.08,0)}\n  {prop("Value","Q_NPN_BCE",-0.508,-5.08,0)}\n  {prop("Footprint","",hide=True)}\n  {prop("Datasheet","~",hide=True)}\n'
    s += f'  (symbol "Q_NPN_BCE_0_1"\n    {poly((0.508,0),(2.032,0))}\n    {poly((2.032,-2.54),(2.032,2.54))}\n    {poly((2.032,1.27),(4.572,2.794))}\n    {poly((2.032,-1.27),(4.572,-2.794))}\n    {poly((3.302,-2.032),(4.572,-2.794),(4.064,-1.524))})\n'
    s += f'  (symbol "Q_NPN_BCE_1_1"\n    {pdef("B","B",0,0,0,"input")}\n    {pdef("C","C",5.08,2.54,270,"passive",0.508)}\n    {pdef("E","E",5.08,-2.54,90,"passive",0.508)})\n)\n'

    # NMOS
    s += f'(symbol "Device:Q_NMOS_GDS"\n  (pin_names(offset 1.016))\n  (exclude_from_sim no)(in_bom yes)(on_board yes)\n'
    s += f'  {prop("Reference","Q",5.08,5.08,0)}\n  {prop("Value","Q_NMOS_GDS",-0.508,-5.08,0)}\n  {prop("Footprint","",hide=True)}\n  {prop("Datasheet","~",hide=True)}\n'
    s += f'  (symbol "Q_NMOS_GDS_0_1"\n    {poly((0,0),(1.524,0))}\n    {poly((1.524,-2.032),(1.524,2.032))}\n    {poly((2.032,1.524),(2.032,2.794))}\n    {poly((2.032,-1.524),(2.032,-2.794))}\n    {poly((2.032,2.794),(4.572,2.794))}\n    {poly((2.032,-2.794),(4.572,-2.794))}\n    {poly((3.302,-2.032),(4.572,-2.794),(3.302,-2.794))})\n'
    s += f'  (symbol "Q_NMOS_GDS_1_1"\n    {pdef("G","G",0,0,0,"input")}\n    {pdef("D","D",5.08,2.54,270,"passive",0.508)}\n    {pdef("S","S",5.08,-2.54,90,"passive",0.508)})\n)\n'

    # Battery
    s += f'(symbol "Device:Battery_Cell"\n  (pin_numbers(hide yes))\n  (exclude_from_sim no)(in_bom yes)(on_board yes)\n'
    s += f'  {prop("Reference","BT",0.762,0,0)}\n  {prop("Value","Battery",0,-3.81,0)}\n  {prop("Footprint","",hide=True)}\n  {prop("Datasheet","~",hide=True)}\n'
    s += f'  (symbol "Battery_Cell_0_1"\n    {poly((-2.032,0.762),(2.032,0.762))}\n    {poly((-1.016,-0.762),(1.016,-0.762))}\n    {poly((0,0.762),(0,2.54))}\n    {poly((0,-0.762),(0,-2.54))})\n'
    s += f'  (symbol "Battery_Cell_1_1"\n    {pdef("+","+",0,2.54,270,"power_in")}\n    {pdef("-","-",0,-2.54,90,"power_in")})\n)\n'

    # SW_SPST
    s += f'(symbol "Switch:SW_SPST"\n  (pin_numbers(hide yes))\n  (exclude_from_sim no)(in_bom yes)(on_board yes)\n'
    s += f'  {prop("Reference","SW",0,2.032,0)}\n  {prop("Value","SW_SPST",0,-2.032,0)}\n  {prop("Footprint","",hide=True)}\n  {prop("Datasheet","~",hide=True)}\n'
    s += f'  (symbol "SW_SPST_0_1"\n    {poly((-3.81,0),(-2.032,0))}\n    {poly((2.032,0),(3.81,0))}\n    {poly((-2.032,0.508),(2.032,1.524))})\n'
    s += f'  (symbol "SW_SPST_1_1"\n    {pdef(1,"A",-3.81,0,0)}\n    {pdef(2,"B",3.81,0,180)})\n)\n'

    # SW_Push
    s += f'(symbol "Switch:SW_Push"\n  (pin_numbers(hide yes))\n  (exclude_from_sim no)(in_bom yes)(on_board yes)\n'
    s += f'  {prop("Reference","SW",0,2.032,0)}\n  {prop("Value","SW_Push",0,-2.032,0)}\n  {prop("Footprint","",hide=True)}\n  {prop("Datasheet","~",hide=True)}\n'
    s += f'  (symbol "SW_Push_0_1"\n    {poly((-2.032,0),(-1.016,0))}\n    {poly((1.016,0),(2.032,0))}\n    {poly((-1.016,1.016),(1.016,1.016))})\n'
    s += f'  (symbol "SW_Push_1_1"\n    {pdef(1,"A",-2.032,0,0)}\n    {pdef(2,"B",2.032,0,180)})\n)\n'

    # Buzzer
    s += f'(symbol "Device:Buzzer"\n  (pin_names(offset 1.016))\n  (exclude_from_sim no)(in_bom yes)(on_board yes)\n'
    s += f'  {prop("Reference","BZ",3.302,0,0)}\n  {prop("Value","Buzzer",0,-3.302,0)}\n  {prop("Footprint","",hide=True)}\n  {prop("Datasheet","~",hide=True)}\n'
    s += f'  (symbol "Buzzer_0_1"\n    (circle (center 0 0) (radius 2.286) (stroke (width 0)(type default))(fill(type none)))\n    {poly((-0.762,0.762),(0.762,0.762))}\n    {poly((0,0.762),(0,2.286))})\n'
    s += f'  (symbol "Buzzer_1_1"\n    {pdef("K","K",0,2.286,270,"passive",0)}\n    {pdef("+","+",  -3.81,0,0,"passive",1.524)})\n)\n'

    # Solenoid
    s += f'(symbol "Mechanical:Solenoid"\n  (pin_names(offset 1.016))\n  (exclude_from_sim no)(in_bom yes)(on_board yes)\n'
    s += f'  {prop("Reference","SOL",3.81,0,0)}\n  {prop("Value","Solenoid",0,-3.81,0)}\n  {prop("Footprint","",hide=True)}\n  {prop("Datasheet","~",hide=True)}\n'
    s += f'  (symbol "Solenoid_0_1"\n    {rect(-2.54,-2.54,2.54,2.54)}\n    {poly((-1.27,-2.54),(-1.27,2.54))}\n    {poly((1.27,-2.54),(1.27,2.54))})\n'
    s += f'  (symbol "Solenoid_1_1"\n    {pdef(1,"1",0,3.81,270,"passive",1.27)}\n    {pdef(2,"2",0,-3.81,90,"passive",1.27)})\n)\n'

    # PIC18LF25K22 (28-DIP)
    s += f'(symbol "MCU:PIC18LF25K22"\n  (exclude_from_sim no)(in_bom yes)(on_board yes)\n'
    s += f'  {prop("Reference","U",8.89,0,90)}\n  {prop("Value","PIC18LF25K22",-8.89,0,90)}\n  {prop("Footprint","Package_DIP:DIP-28_W7.62mm",hide=True)}\n  {prop("Datasheet","~",hide=True)}\n'
    s += f'  (symbol "PIC18LF25K22_0_1" {rect(-7.62,-17.78,7.62,17.78)})\n'
    s += '  (symbol "PIC18LF25K22_1_1"\n'
    lp=[('MCLR','input'),('RA0','bidirectional'),('RA1','bidirectional'),('RA2','bidirectional'),
        ('RA3','bidirectional'),('RA4','bidirectional'),('RA5','bidirectional'),('VSS','power_in'),
        ('RA7','bidirectional'),('RA6','bidirectional'),('RC0','bidirectional'),('RC1','bidirectional'),
        ('RC2','bidirectional'),('RC3','bidirectional')]
    rp=[('RC4','bidirectional'),('RC5','bidirectional'),('RC6','bidirectional'),('RC7','bidirectional'),
        ('VSS','power_in'),('VDD','power_in'),('RB0','bidirectional'),('RB1','bidirectional'),
        ('RB2','bidirectional'),('RB3','bidirectional'),('RB4','bidirectional'),('RB5','bidirectional'),
        ('RB6','bidirectional'),('RB7','bidirectional')]
    for i,(name,pt) in enumerate(lp):
        py=16.51-i*2.54; n=i+1
        s += f'    {pdef(n,name,-7.62,py,0,pt)}\n'
    for i,(name,pt) in enumerate(rp):
        py=-16.51+i*2.54; n=i+15
        s += f'    {pdef(n,name,7.62,py,180,pt)}\n'
    s += '  )\n)\n'

    # MPU-6050
    s += f'(symbol "Sensor_Motion:MPU-6050"\n  (exclude_from_sim no)(in_bom yes)(on_board yes)\n'
    s += f'  {prop("Reference","U",7.62,0,90)}\n  {prop("Value","MPU-6050",-7.62,0,90)}\n  {prop("Footprint","",hide=True)}\n  {prop("Datasheet","~",hide=True)}\n'
    s += f'  (symbol "MPU-6050_0_1" {rect(-5.08,-5.08,5.08,5.08)})\n'
    s += '  (symbol "MPU-6050_1_1"\n'
    ml=[('VCC','power_in'),('GND','power_in'),('SCL','input'),('SDA','bidirectional')]
    mr=[('INT','output'),('AD0','input'),('XCL','bidirectional'),('XDA','bidirectional')]
    for i,(name,pt) in enumerate(ml):
        py=5.08-i*2.54; n=i+1
        s += f'    {pdef(n,name,-7.62,py,0,pt)}\n'
    for i,(name,pt) in enumerate(mr):
        py=5.08-i*2.54; n=i+5
        s += f'    {pdef(n,name,7.62,py,180,pt)}\n'
    s += '  )\n)\n'

    # MCP1700
    s += f'(symbol "Regulator_Linear:MCP1700-3302E"\n  (exclude_from_sim no)(in_bom yes)(on_board yes)\n'
    s += f'  {prop("Reference","U",5.08,2.54,0)}\n  {prop("Value","MCP1700-3302E",5.08,-2.54,0)}\n  {prop("Footprint","Package_TO_SOT_SMD:SOT-23",hide=True)}\n  {prop("Datasheet","~",hide=True)}\n'
    s += f'  (symbol "MCP1700-3302E_0_1" {rect(-2.54,-3.81,2.54,3.81)})\n'
    s += f'  (symbol "MCP1700-3302E_1_1"\n    {pdef(1,"IN",-5.08,1.27,0,"power_in")}\n    {pdef(2,"GND",-5.08,-1.27,0,"power_in")}\n    {pdef(3,"OUT",5.08,0,180,"power_out")})\n)\n'

    # MT3608
    s += f'(symbol "Regulator_Switching:MT3608"\n  (exclude_from_sim no)(in_bom yes)(on_board yes)\n'
    s += f'  {prop("Reference","U",5.08,6.35,0)}\n  {prop("Value","MT3608",5.08,-6.35,0)}\n  {prop("Footprint","Package_TO_SOT_SMD:SOT-23-6",hide=True)}\n  {prop("Datasheet","~",hide=True)}\n'
    s += f'  (symbol "MT3608_0_1" {rect(-2.54,-5.08,2.54,5.08)})\n'
    s += f'  (symbol "MT3608_1_1"\n    {pdef(1,"IN",-5.08,3.81,0,"power_in")}\n    {pdef(2,"GND",-5.08,1.27,0,"power_in")}\n    {pdef(3,"FB",-5.08,-1.27,0,"input")}\n    {pdef(4,"EN",-5.08,-3.81,0,"input")}\n    {pdef(5,"SW",5.08,3.81,180,"output")}\n    {pdef(6,"BS",5.08,-3.81,180,"passive")})\n)\n'

    # Conn_01x04
    s += f'(symbol "Connector_Generic:Conn_01x04"\n  (exclude_from_sim no)(in_bom yes)(on_board yes)\n'
    s += f'  {prop("Reference","J",3.81,3.81,0)}\n  {prop("Value","Conn_01x04",3.81,-3.81,0)}\n  {prop("Footprint","Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical",hide=True)}\n  {prop("Datasheet","~",hide=True)}\n'
    s += f'  (symbol "Conn_01x04_0_1" {rect(-1.016,-5.08,0,5.08)})\n'
    s += f'  (symbol "Conn_01x04_1_1"\n    {pdef(1,"Pin_1",0,3.81,180)}\n    {pdef(2,"Pin_2",0,1.27,180)}\n    {pdef(3,"Pin_3",0,-1.27,180)}\n    {pdef(4,"Pin_4",0,-3.81,180)})\n)\n'

    # Power symbols — (power) flag required for kicad-happy to classify as power_symbol
    for name in ['GND','+3V3','+5V','VBAT','VBAT_SW','BOOST_SW','BOOST_FB']:
        s += f'(symbol "power:{name}"\n  (power)\n  (exclude_from_sim no)(in_bom yes)(on_board yes)\n'
        s += f'  {prop("Reference","#PWR",0,-1.524,0,1.27,True)}\n  {prop("Value",name,0,3.556,0)}\n'
        if name == 'GND':
            s += f'  (symbol "GND_0_1"\n    {poly((0,0),(0,-1.27))}\n    {poly((-1.27,-1.27),(1.27,-1.27))}\n    {poly((-0.762,-1.905),(0.762,-1.905))})\n'
            s += f'  (symbol "GND_1_1"\n    {pdef(1,"PWR",0,0,270,"power_in",0)})\n)\n'
        else:
            s += f'  (symbol "{name}_0_1"\n    {poly((0,0),(0,1.27))}\n    {poly((-0.762,1.27),(0.762,1.27))})\n'
            s += f'  (symbol "{name}_1_1"\n    {pdef(1,"PWR",0,0,90,"power_in",0)})\n)\n'

    s += ')\n'
    return s

# ── Place component ───────────────────────────────────────────────────────────
def place_comp(lib_id, ref, val, cx, cy, angle, pin_map, footprint=''):
    lines = [f'(symbol (lib_id {esc(lib_id)}) (at {cx} {cy} {angle}) (unit 1)']
    lines.append('  (exclude_from_sim no)(in_bom yes)(on_board yes)')
    lines.append(f'  (uuid {esc(u())})')
    lines.append(f'  (property "Reference" {esc(ref)} (at {cx+3} {cy-3} 0) (effects (font (size 1.27 1.27))))')
    lines.append(f'  (property "Value" {esc(val)} (at {cx+3} {cy+3} 0) (effects (font (size 1.27 1.27))))')
    lines.append(f'  (property "Footprint" {esc(footprint)} (at {cx} {cy} 0) (effects (font (size 1.27 1.27)) (hide yes)))')
    lines.append(f'  (property "Datasheet" "" (at {cx} {cy} 0) (effects (font (size 1.27 1.27)) (hide yes)))')
    # Only emit each unique pin name/number once, prefer names over numbers
    seen_pos = set()
    for pname, (px,py) in pin_map.items():
        pos = (px,py)
        if pos not in seen_pos:
            lines.append(f'  (pin {esc(str(pname))} (uuid {esc(u())}))')
            seen_pos.add(pos)
    lines.append(')\n')
    return '\n'.join(lines)

_pwr_counter = 0

def place_power(name, x, y, angle=0):
    global _pwr_counter
    _pwr_counter += 1
    ref = f'#PWR{_pwr_counter:03d}'
    # Must use (symbol ...) not (power ...) — analyzer only reads (symbol ...) elements
    return (f'(symbol (lib_id {esc("power:"+name)}) (at {x} {y} {angle}) (unit 1)\n'
            f'  (exclude_from_sim no)(in_bom yes)(on_board yes)\n'
            f'  (uuid {esc(u())})\n'
            f'  (property "Reference" {esc(ref)} (at {x} {y-1.5} 0) (effects (font (size 1.27 1.27)) (hide yes)))\n'
            f'  (property "Value" {esc(name)} (at {x} {y+2} 0) (effects (font (size 1.27 1.27))))\n'
            f'  (pin "1" (uuid {esc(u())}))\n)\n')

def place_label(name, x, y, angle=0):
    return (f'(label {esc(name)} (at {x} {y} {angle})\n'
            f'  (effects (font (size 1.27 1.27)))\n'
            f'  (uuid {esc(u())}))\n')

def place_nc(x, y):
    return f'(no_connect (at {x} {y}) (uuid {esc(u())}))\n'

# ── Build ─────────────────────────────────────────────────────────────────────
def build():
    parts = [lib_symbols()]

    # Build comp lookup
    comp_map = {}
    for comp in COMPS:
        lib_id,ref,val,cx,cy,angle,pmap,*fp = comp
        footprint = fp[0] if fp else ''
        comp_map[ref] = comp
        parts.append(place_comp(lib_id,ref,val,cx,cy,angle,pmap,footprint))

    # Net labels / power symbols at exact pin positions
    POWER_NETS = {'+3V3','+5V','GND','VBAT','VBAT_SW','BOOST_SW','BOOST_FB'}
    for net_name, pin_list in NETS.items():
        for ref, pname in pin_list:
            if ref not in comp_map:
                print(f'WARN: {ref} not found'); continue
            _,_,_,cx,cy,angle,pmap,*_ = comp_map[ref]
            if pname not in pmap:
                print(f'WARN: {ref}.{pname} not in pmap (available: {list(pmap)[:5]})'); continue
            px,py = pmap[pname]
            sx,sy = pin_pos(cx,cy,angle,px,py)
            if net_name in POWER_NETS:
                parts.append(place_power(net_name, sx, sy))
            else:
                parts.append(place_label(net_name, sx, sy))

    # No-connects
    for ref, pname in NC:
        _,_,_,cx,cy,angle,pmap,*_ = comp_map[ref]
        if pname not in pmap: continue
        px,py = pmap[pname]
        sx,sy = pin_pos(cx,cy,angle,px,py)
        parts.append(place_nc(sx,sy))

    parts.append('(sheet_instances (path "/" (page "1")))\n')
    return ''.join(parts)

# ── Output ────────────────────────────────────────────────────────────────────
header = f'(kicad_sch\n\t(version 20260306)\n\t(generator "eeschema")\n\t(generator_version "10.0")\n\t(uuid {esc(u())})\n\t(paper "A4")\n'
content = header + build() + ')\n'

out = 'hardware/paracaidas-matrice400/paracaidas-matrice400.kicad_sch'
with open(out,'w',encoding='utf-8') as f:
    f.write(content)
print(f"Generated: {out}  ({len(content):,} bytes)")
