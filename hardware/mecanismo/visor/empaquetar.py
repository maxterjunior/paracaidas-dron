"""Regenera visor/piezas.js a partir de mecanismo.scad.

Exporta cada pieza con OpenSCAD, la pasa a STL binario y la guarda en base64
dentro de piezas.js (window.PIEZAS_STL), que index.html carga con <script>.
Así el visor funciona abriendo index.html directamente y como artifact.

Uso:  python empaquetar.py "C:/Program Files/OpenSCAD/openscad.com"
"""
import base64, pathlib, re, struct, subprocess, sys, tempfile

PIEZAS = ["adaptador", "caja", "tapa_caja", "base", "tubo", "collar", "solenoide", "piston",
          "paracaidas", "resorte", "embolo", "palanca", "tapa", "dron", "helices"]
AQUI = pathlib.Path(__file__).resolve().parent
SCAD = AQUI.parent / "mecanismo.scad"
TRI = re.compile(r"facet normal\s+(\S+)\s+(\S+)\s+(\S+)\s+outer loop" + r"\s+vertex\s+(\S+)\s+(\S+)\s+(\S+)" * 3)


def stl_binario(texto: str) -> bytes:
    tris = TRI.findall(texto)
    out = [b"mecanismo.scad".ljust(80, b" "), struct.pack("<I", len(tris))]
    out += [struct.pack("<12fH", *map(float, t), 0) for t in tris]
    return b"".join(out)


def main(openscad: str) -> None:
    datos = {}
    with tempfile.TemporaryDirectory() as tmp:
        for p in PIEZAS:
            stl = pathlib.Path(tmp) / f"{p}.stl"
            r = subprocess.run([openscad, "-D", f'pieza="{p}"', "-o", str(stl), str(SCAD)],
                               capture_output=True, text=True)
            if r.returncode or not stl.exists():
                sys.exit(f"{p}: OpenSCAD falló\n{r.stderr}")
            datos[p] = base64.b64encode(stl_binario(stl.read_text())).decode()
            print(f"{p:11s} {len(datos[p]) // 1024:5d} KB")
    js = "// Generado por empaquetar.py a partir de mecanismo.scad. No editar a mano.\nwindow.PIEZAS_STL = {\n"
    js += ",\n".join(f'  "{k}": "{v}"' for k, v in datos.items()) + "\n};\n"
    (AQUI / "piezas.js").write_text(js, encoding="ascii")


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "openscad")
