// =============================================================================
// Mecanismo de paracaídas (tubo de muelle) para DJI Matrice 400
// Modelo paramétrico OpenSCAD. Unidades: mm. Eje Z hacia arriba, eje del tubo en Z.
//
// Funcionamiento:
//   - El resorte empuja el pistón; el paracaídas plegado va sobre el pistón.
//   - La tapa está articulada (bisagra en -X) y retenida en +X por una pestaña
//     con ventana en la que entra el gancho de la palanca (pestillo).
//   - La carga del resorte cae casi sobre el pivote de la palanca (brazo e ≈ 2.7 mm),
//     así que el émbolo del solenoide sólo bloquea la palanca de lado (en cortante),
//     con pocos newtons. Sin energía el muelle del solenoide mantiene el émbolo
//     extendido = cerrado. Pulso de ~300 ms → el émbolo se retrae → la palanca
//     gira hacia afuera → la tapa se abre → el resorte expulsa el paracaídas.
//   - La tapa queda unida por la bisagra (no cae nada suelto desde el dron).
//
// Uso:  openscad -D 'pieza="base"' -o base.stl mecanismo.scad
//   piezas imprimibles: base, tubo, piston, tapa, collar, palanca, caja, tapa_caja, adaptador
//   piezas de referencia (no se imprimen): resorte, solenoide, embolo, paracaidas, dron
//   "conjunto" = todo montado sobre el dron; "mecanismo" = sin el dron
//   Todas las piezas salen en su posición de montaje; orientarlas en el slicer.
// =============================================================================

pieza = "conjunto";

$fn = 96;

// ---------------- Tubo ----------------
tubo_di    = 100;     // diámetro interior (1.5–2 L de paracaídas plegado)
tubo_pared = 2.5;     // impreso PETG/ASA; alternativa: tubo PVC 110 mm recortado
tubo_l     = 280;
tubo_de    = tubo_di + 2*tubo_pared;   // 105
holg       = 0.2;     // holgura de ajuste para piezas que abrazan el tubo

// ---------------- Base ----------------
base_lado  = 130;     // placa cuadrada inferior
base_e     = 5;       // espesor placa
base_piso  = 10;      // el tubo apoya en z = base_piso
base_cuello= 15;      // profundidad del encaje del tubo
base_tornillo_sep = 112;   // 4× M4 en cuadrado
anclaje_d  = 10;      // paso del cordón de anclaje (bridle) por el centro

// ---------------- Resorte (referencia) ----------------
resorte_de     = 90;   // diámetro exterior
resorte_alambre= 2.5;
resorte_espiras= 7;
resorte_comp   = 45;   // largo comprimido (montado)
resorte_libre  = 260;  // largo libre (expulsa el pistón hasta la boca del tubo)

// ---------------- Pistón ----------------
piston_e     = 4;
piston_falda = 12;

// ---------------- Solenoide (referencia, tipo JF-0530B 5 V push-pull) ----------------
// VERIFICAR con el solenoide real: cuerpo, carrera y largo del émbolo.
sol_cuerpo = [13, 30, 16];   // X, Y (eje del émbolo), Z
sol_embolo_d = 6;
sol_carrera  = 10;

// ---------------- Caja electrónica ----------------
pcb = [60, 40, 1.6];
pcb_agujeros = [[3.5, 3.5], [56.5, 3.5], [3.5, 36.5], [56.5, 36.5]];  // coord. locales del PCB (y hacia abajo en KiCad)
caja_int   = [66, 86, 26];   // PCB (lado +Y) + bahía de batería LiPo 1S (lado -Y)
caja_pared = 2;
caja_sep   = 14;             // altura de los separadores del PCB
caja_c     = [0, -112];      // centro de la caja sobre el adaptador

// ---------------- Adaptador al dron ----------------
adap_e  = 4;
adap_x  = [-70, 70];
adap_y  = [-162, 70];
// Patrón de fijación al dron: PLACEHOLDER. Medir el punto de montaje real del M400.
montaje_dx = 100;
montaje_dy = 180;
montaje_d  = 4.5;   // M4

// ---------------- Dron (envolvente aproximada, sólo visual) ----------------
// VERIFICAR contra las medidas del Matrice 400.
dron_cuerpo = [360, 280, 140];
dron_eje    = 1070;   // distancia diagonal entre motores
helice_d    = 660;
helice_z    = 20;     // plano de hélices respecto a la cara superior del dron

// ---------------- Cotas derivadas ----------------
zt = base_piso + tubo_l;           // boca del tubo
r_t = tubo_de/2;                   // radio exterior del tubo (52.5)
bis = [-58, zt + 2];               // eje de la bisagra (x, z), paralelo a Y
piv = [59, zt - 62];               // eje del pivote de la palanca (x, z), paralelo a Y
emb = [65, zt - 42]   ;             // eje del émbolo (x, z), paralelo a Y
collar_z = [zt - 90, zt - 66];
piston_z = base_piso + resorte_comp;

// =============================================================================
// Utilidades
// =============================================================================
module eje_y(p, r, l) { translate([p[0], 0, p[1]]) rotate([90, 0, 0]) cylinder(r = r, h = l, center = true); }
module abraza_tubo() { difference() { children(); translate([0, 0, -1]) cylinder(r = r_t + holg, h = zt + 20); } }
module caja_xyz(x, y, z) { translate([x[0], y[0], z[0]]) cube([x[1]-x[0], y[1]-y[0], z[1]-z[0]]); }

// =============================================================================
// Piezas
// =============================================================================
module tubo() {
    difference() {
        translate([0, 0, base_piso]) cylinder(d = tubo_de, h = tubo_l);
        translate([0, 0, base_piso - 1]) cylinder(d = tubo_di, h = tubo_l + 2);
    }
}

module base() {
    difference() {
        union() {
            translate([-base_lado/2, -base_lado/2, 0]) cube([base_lado, base_lado, base_e]);
            cylinder(d = tubo_de + 2*holg + 8, h = base_piso + base_cuello);
            // nervios
            for (a = [0:90:270]) rotate(a) translate([r_t, -2, base_e]) cube([base_lado/2 - r_t - 1, 4, 12]);
        }
        // encaje del tubo
        translate([0, 0, base_piso]) cylinder(d = tubo_de + 2*holg, h = 50);
        // anclaje del cordón
        translate([0, 0, -1]) cylinder(d = anclaje_d, h = 50);
        // tornillos M4 con cabeza embutida desde arriba
        for (sx = [-1, 1], sy = [-1, 1]) translate([sx*base_tornillo_sep/2, sy*base_tornillo_sep/2, -1]) {
            cylinder(d = 4.4, h = 20);
            translate([0, 0, base_e + 1]) cylinder(d = 8.5, h = 20);
        }
    }
    // asiento centrador del resorte
    translate([0, 0, base_piso]) difference() {
        cylinder(r = resorte_de/2 - resorte_alambre - 0.5, h = 5);
        translate([0, 0, -1]) cylinder(r = resorte_de/2 - resorte_alambre - 3, h = 7);
    }
}

module piston() {
    translate([0, 0, piston_z]) difference() {
        union() {
            cylinder(d = tubo_di - 1, h = piston_e);
            translate([0, 0, -piston_falda]) difference() {
                cylinder(d = tubo_di - 1, h = piston_falda);
                translate([0, 0, -1]) cylinder(d = tubo_di - 5, h = piston_falda + 2);
            }
        }
        translate([0, 0, -1]) cylinder(d = anclaje_d, h = 20);
        // ranuras para pasar cuerdas sin tensar
        for (a = [45, 225]) rotate(a) translate([30, 0, -1]) cylinder(d = 6, h = 20);
    }
}

module tapa() {
    difference() {
        union() {
            translate([0, 0, zt]) cylinder(d = tubo_de + 2, h = 3);
            // labio centrador interior
            translate([0, 0, zt - 8]) difference() {
                cylinder(d = tubo_di - 0.8, h = 8);
                translate([0, 0, -1]) cylinder(d = tubo_di - 5, h = 10);
            }
            // nudillo central de la bisagra
            hull() { eje_y(bis, 4, 9); caja_xyz([bis[0], -53], [-4.5, 4.5], [zt, zt + 3]); }   // no baja de zt dentro de r_t
            // pestaña del pestillo con ventana
            caja_xyz([r_t + 0.3, r_t + 3.3], [-8, 8], [zt - 45, zt + 3]);
        }
        eje_y(bis, 1.65, 20);                                     // pasador M3
        caja_xyz([r_t, r_t + 5], [-5, 5], [zt - 40, zt - 33]);    // ventana
    }
}

module collar() {
    difference() {
        abraza_tubo() union() {
            translate([0, 0, collar_z[0]]) cylinder(r = r_t + 4.5, h = collar_z[1] - collar_z[0]);
            // horquilla de la bisagra (dos dedos a y = ±5..12)
            for (s = [-1, 1]) hull() {
                caja_xyz([-62, -48], s > 0 ? [5, 12] : [-12, -5], [collar_z[0], collar_z[1]]);
                translate([0, s*8.5, 0]) eje_y(bis, 4, 7);
            }
            // orejas del pivote de la palanca (y = ±3.5..7)
            for (s = [-1, 1]) caja_xyz([45, 64], s > 0 ? [3.5, 7] : [-7, -3.5], [collar_z[0], piv[1] + 6]);
            // guía del émbolo
            caja_xyz([58, 70], [5, 11], [collar_z[0], emb[1] + 6]);
            // repisa del solenoide
            caja_xyz([30, 76], [11, 44], [collar_z[0], emb[1] - sol_cuerpo[2]/2]);
        }
        eje_y(bis, 1.65, 40);                                   // pasador M3 de la bisagra
        eje_y(piv, 1.05, 30);                                   // pasador Ø2 del pivote
        translate([emb[0], 0, emb[1]]) rotate([-90, 0, 0]) cylinder(d = sol_embolo_d + 0.6, h = 12);
        // ranuras verticales para bridas que abrazan el solenoide
        for (y = [18, 36], x = [56.2, 72.4]) caja_xyz([x, x + 2.2], [y - 2, y + 2], [collar_z[0] - 1, zt]);
    }
}

module palanca() {
    difference() {
        union() {
            caja_xyz([r_t + 3.8, r_t + 9.2], [-3, 3], [zt - 65, zt - 32]);
            // gancho que entra en la ventana de la pestaña
            caja_xyz([r_t + 0.8, r_t + 4], [-3, 3], [zt - 39, zt - 34]);
        }
        eje_y(piv, 1.1, 10);
    }
}

// ---------------- Referencias ----------------
module resorte(largo = resorte_comp) {
    translate([0, 0, base_piso])
        linear_extrude(height = largo, twist = -360*resorte_espiras, slices = resorte_espiras*24)
            translate([resorte_de/2 - resorte_alambre/2, 0]) circle(d = resorte_alambre, $fn = 12);
}

module solenoide() {
    translate([emb[0] - sol_cuerpo[0]/2, 12, emb[1] - sol_cuerpo[2]/2]) cube(sol_cuerpo);
}

module embolo(retraido = false) {
    // extendido: punta en y = -5 (bloquea la palanca, que ocupa y = ±3)
    translate([emb[0], retraido ? sol_carrera : 0, emb[1]]) rotate([-90, 0, 0])
        translate([0, 0, -5]) cylinder(d = sol_embolo_d, h = 12 + 5 + 10);
}

module paracaidas() {
    translate([0, 0, piston_z + piston_e]) cylinder(d = tubo_di - 4, h = zt - 9 - piston_z - piston_e);
}

// ---------------- Caja electrónica ----------------
function caja_ext() = [caja_int[0] + 2*caja_pared, caja_int[1] + 2*caja_pared, caja_int[2] + caja_pared];
// origen del PCB (esquina superior izquierda en KiCad) dentro de la caja; y KiCad hacia -Y
function pcb_o() = [caja_c[0] - pcb[0]/2, caja_c[1] + caja_int[1]/2 - 3];
function pcb_xy(p) = [pcb_o()[0] + p[0], pcb_o()[1] - p[1]];

module caja() {
    e = caja_ext();
    translate([0, 0, 0]) difference() {
        translate([caja_c[0] - e[0]/2, caja_c[1] - e[1]/2, 0]) cube(e);
        translate([caja_c[0] - caja_int[0]/2, caja_c[1] - caja_int[1]/2, caja_pared]) cube([caja_int[0], caja_int[1], 50]);
        // tornillos M3 a la placa adaptadora
        for (sx = [-1, 1], sy = [-1, 1]) translate([caja_c[0] + sx*15, caja_c[1] + sy*20, -1]) cylinder(d = 3.4, h = 10);
        // SW1 (deslizante, borde izquierdo del PCB, y = 22.5)
        let(p = pcb_xy([0, 22.5])) caja_xyz([p[0] - 10, p[0] + 1], [p[1] - 5, p[1] + 5], [caja_pared + caja_sep - 1, caja_pared + caja_sep + 5]);
        // cable del solenoide (J4, borde derecho, y = 24) y del disparo RC
        let(p = pcb_xy([60, 24])) translate([p[0], p[1], caja_pared + caja_sep + 4]) rotate([0, 90, 0]) cylinder(d = 6, h = 20);
        let(p = pcb_xy([60, 34])) translate([p[0], p[1], caja_pared + caja_sep + 4]) rotate([0, 90, 0]) cylinder(d = 5, h = 20);
    }
    // separadores del PCB
    for (h = pcb_agujeros) let(p = pcb_xy(h)) translate([p[0], p[1], caja_pared - 0.5]) difference() {
        cylinder(d = 6, h = caja_sep + 0.5, $fn = 32);
        cylinder(d = 2.5, h = caja_sep + 1, $fn = 24);   // M3 autorroscante
    }
    // tope de la batería
    translate([caja_c[0] - caja_int[0]/2, caja_c[1] - caja_int[1]/2 + 40, caja_pared - 0.5]) cube([caja_int[0], 1.5, 8.5]);
    // tetones para los tornillos de la tapa
    for (sx = [-1, 1], sy = [-1, 1]) translate([caja_c[0] + sx*(caja_int[0]/2 - 2.5), caja_c[1] + sy*(caja_int[1]/2 - 2.5), caja_pared - 0.5])
        difference() { cylinder(d = 6, h = caja_int[2] + 0.5, $fn = 32); cylinder(d = 2.5, h = caja_int[2] + 1, $fn = 24); }
}

module tapa_caja() {
    e = caja_ext();
    z0 = e[2];
    difference() {
        translate([caja_c[0] - e[0]/2, caja_c[1] - e[1]/2, z0]) cube([e[0], e[1], 2]);
        for (sx = [-1, 1], sy = [-1, 1]) translate([caja_c[0] + sx*(caja_int[0]/2 - 2.5), caja_c[1] + sy*(caja_int[1]/2 - 2.5), z0 - 5]) cylinder(d = 3.4, h = 10, $fn = 24);
        // LED1, LED2 (guías de luz Ø3), buzzer, reset SW2
        for (h = [[22, 23.5, 3], [22, 25.6, 3], [43.5, 34.2, 4], [20.5, 10.5, 3.5]]) let(p = pcb_xy(h))
            translate([p[0], p[1], z0 - 5]) cylinder(d = h[2], h = 10, $fn = 24);
        // acceso a J1 (UART) y J2 (ICSP) sin abrir la caja
        let(p = pcb_xy([30, 3.6])) caja_xyz([p[0] - 9, p[0] + 9], [p[1] - 2.5, p[1] + 2.5], [z0 - 5, z0 + 5]);
        let(p = pcb_xy([56.5, 12])) caja_xyz([p[0] - 2.5, p[0] + 2.5], [p[1] - 5, p[1] + 5], [z0 - 5, z0 + 5]);
    }
}

module adaptador() {
    difference() {
        caja_xyz(adap_x, adap_y, [-adap_e, 0]);
        // base del tubo M4
        for (sx = [-1, 1], sy = [-1, 1]) translate([sx*base_tornillo_sep/2, sy*base_tornillo_sep/2, -10]) cylinder(d = 4.4, h = 20);
        translate([0, 0, -10]) cylinder(d = anclaje_d + 2, h = 20);
        // caja M3
        for (sx = [-1, 1], sy = [-1, 1]) translate([caja_c[0] + sx*15, caja_c[1] + sy*20, -10]) cylinder(d = 3.4, h = 20);
        // fijación al dron (placeholder)
        cy = (adap_y[0] + adap_y[1])/2;
        for (sx = [-1, 1], sy = [-1, 1]) translate([sx*montaje_dx/2, cy + sy*montaje_dy/2, -10]) cylinder(d = montaje_d, h = 20);
        // aligerado
        for (sx = [-1, 1]) translate([sx*30, -40, -10]) cylinder(d = 26, h = 20);
    }
}

// ---------------- Dron (envolvente) ----------------
module dron() {
    zc = -adap_e;
    translate([0, 0, zc - dron_cuerpo[2]]) hull() for (sx = [-1, 1], sy = [-1, 1])
        translate([sx*(dron_cuerpo[0]/2 - 30), sy*(dron_cuerpo[1]/2 - 30), 0]) cylinder(r = 30, h = dron_cuerpo[2]);
    for (a = [45:90:315]) rotate(a) {
        translate([0, 0, zc - 40]) rotate([0, 90, 0]) cylinder(d = 40, h = dron_eje/2, $fn = 32);
        translate([dron_eje/2, 0, zc - 60]) cylinder(d = 70, h = helice_z + 50 + adap_e, $fn = 48);
        translate([dron_eje/2, 0, helice_z]) cylinder(d = helice_d, h = 2, $fn = 96);
    }
    // tren de aterrizaje
    for (sx = [-1, 1]) {
        translate([sx*140, 0, zc - dron_cuerpo[2] - 230]) rotate([90, 0, 0]) cylinder(d = 20, h = 500, center = true, $fn = 24);
        for (sy = [-1, 1]) hull() {
            translate([sx*110, sy*90, zc - dron_cuerpo[2] + 5]) sphere(d = 18, $fn = 24);
            translate([sx*140, sy*150, zc - dron_cuerpo[2] - 230]) sphere(d = 18, $fn = 24);
        }
    }
}

// =============================================================================
// Selección
// =============================================================================
module mecanismo() {
    color("DimGray")      base();
    color("WhiteSmoke", 0.45) tubo();
    color("SteelBlue")    piston();
    color("OrangeRed")    tapa();
    color("DimGray")      collar();
    color("Gold")         palanca();
    color("Silver")       resorte();
    color("Black")        solenoide();
    color("Silver")       embolo();
    color("Orange")       paracaidas();
    color("SlateGray")    caja();
    color("LightSlateGray", 0.8) tapa_caja();
    color("Gainsboro")    adaptador();
}

if      (pieza == "conjunto")   { mecanismo(); color("#3a3d42") dron(); }
else if (pieza == "mecanismo")  mecanismo();
else if (pieza == "base")       base();
else if (pieza == "tubo")       tubo();
else if (pieza == "piston")     piston();
else if (pieza == "tapa")       tapa();
else if (pieza == "collar")     collar();
else if (pieza == "palanca")    palanca();
else if (pieza == "caja")       caja();
else if (pieza == "tapa_caja")  tapa_caja();
else if (pieza == "adaptador")  adaptador();
else if (pieza == "resorte")    resorte();
else if (pieza == "solenoide")  solenoide();
else if (pieza == "embolo")     embolo();
else if (pieza == "paracaidas") paracaidas();
else if (pieza == "dron")       dron();
