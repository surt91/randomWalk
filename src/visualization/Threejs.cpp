#include "Threejs.hpp"

Threejs::Threejs(const std::string &filename)
    : filename(filename)
{
    num_vertices = 0;
    /* Write Header */
    header = std::string(   "<html>\n"
                            "<head>\n"
                            "    <meta charset=\"utf-8\"/>\n"
                            "    <script src=\"https://cdnjs.cloudflare.com/ajax/libs/three.js/r83/three.min.js\"></script>\n"
                            "</head>\n"
                            "<body>\n"
                            "<script>\n"
                            "var camera, scene, renderer,\n"
                            "geometry, material, hullMesh, traceMesh, mesh, group;\n"

                            "init();\n"
                            "animate();\n"

                            "function init() {\n"
                            "    scene = new THREE.Scene();\n"

                            "    camera = new THREE.PerspectiveCamera( 75, window.innerWidth / window.innerHeight, 1, 10000 );\n"
                            "    camera.position.z = 2;\n"

                            "    var light = new THREE.AmbientLight( 0x606060 ); // soft white light\n"
                            "    scene.add( light );\n"
                            "    var directionalLight = new THREE.DirectionalLight(0xffffff,1);\n"
                            "    directionalLight.position.set(1, 1, 1).normalize();\n"
                            "    scene.add(directionalLight);\n"
                            "    var trace = new THREE.Geometry();\n"
                            "    var hull = new THREE.Geometry();\n"
                        );
    // in between we define our geometry
    footer = std::string(   "    hull.computeFaceNormals();\n"
                            "    hull.computeVertexNormals();\n"
                            "    // this will only work, because both have the same size\n"
                            "    hull.normalize();\n"
                            "    trace.normalize();\n"
                            "    material = new THREE.MeshPhongMaterial( { color: 0xee0000, transparent: true, opacity: 0.5, shininess: 60 } );\n"
                            "    material.shading = THREE.FlatShading;\n"
                            "    materialTrace = new THREE.MeshPhongMaterial( { color: 0x444444 } );\n"
                            "    materialTrace.shading = THREE.FlatShading;\n"
                            "    hullMesh = new THREE.Mesh( hull, material );\n"
                            "    traceMesh = new THREE.Mesh( trace, materialTrace );\n"
                            "    group = new THREE.Group();\n"
                            "    group.add(hullMesh);\n"
                            "    group.add(traceMesh);\n"
                            "    scene.add( group );\n"
                            "    renderer = new THREE.WebGLRenderer( { alpha: true, antialias: true } );\n"
                            "    renderer.setSize( window.innerWidth, window.innerHeight );\n"
                            "    document.body.appendChild( renderer.domElement );\n"
                            "}\n"
                            "\n"
                            "function animate() {\n"
                            "    requestAnimationFrame( animate );\n"
                            "    render();\n"
                            "}\n"
                            "\n"

                            "function render() {\n"
                            "    group.rotation.x += 0.003;\n"
                            "    group.rotation.y += 0.006;\n"

                            "    renderer.render( scene, camera );\n"
                            "}\n"
                            "</script>\n"
                            "</body>\n"
                            "</html>\n"
                        );
}

void Threejs::box(const double x, const double y, const double z, const double dx, const double dy, const double dz)
{
    //~ var box = new THREE.BoxGeometry(1, 1, 1);
    //~ var boxMesh = new THREE.Mesh(box);
    //~ boxMesh.updateMatrix(); // as needed
    //~ singleGeometry.merge(boxMesh.geometry, boxMesh.matrix);

    //~ buffer << "//placeBox(<" << x << "," << y << "," << z << ">, <" << dx << "," << dy << "," << dz << ">)\n";

    buffer << "var box = new THREE.BoxGeometry( " << dx << "," << dy << "," << dz << " );\n"
              "box.translate( " << x << "," << y << "," << z << " );\n"
              "var boxMesh = new THREE.Mesh(box);\n"
              "boxMesh.updateMatrix();\n"
              "trace.merge(boxMesh.geometry, boxMesh.matrix);\n";
}

void Threejs::polyline(const std::vector<std::vector<double>> &points)
{
    for(size_t i=1; i<points.size(); ++i)
    {
        const double cX1 = points[i-1][0], cX2 = points[i][0];
        const double cY1 = points[i-1][1], cY2 = points[i][1];
        const double cZ1 = points[i-1][2], cZ2 = points[i][2];

        double x = (cX1+cX2)/2;
        double y = (cY1+cY2)/2;
        double z = (cZ1+cZ2)/2;
        double dx = (0.1 + std::abs(cX1-cX2)*2)/2;
        double dy = (0.1 + std::abs(cY1-cY2)*2)/2;
        double dz = (0.1 + std::abs(cZ1-cZ2)*2)/2;

        box(x, y, z, dx, dy, dz);
    }
}

void Threejs::facet(const std::vector<double> &x, const std::vector<double> &y, const std::vector<double> &z)
{
    buffer << "hull.vertices.push( new THREE.Vector3( " << x[0] << "," << x[1] << "," << x[2] << " ) );\n"
           << "hull.vertices.push( new THREE.Vector3( " << y[0] << "," << y[1] << "," << y[2] << " ) );\n"
           << "hull.vertices.push( new THREE.Vector3( " << z[0] << "," << z[1] << "," << z[2] << " ) );\n";

    num_vertices += 3;

    buffer << "hull.faces.push( new THREE.Face3( " << (num_vertices-3) << "," << (num_vertices-2) << "," << (num_vertices-1) << " ) );\n";
}

void Threejs::save()
{
    std::ofstream oss(filename);

    if(!oss.good())
    {
        LOG(LOG_ERROR) << "File can not be opened: " << filename;
        throw std::invalid_argument("cannot be opened");
    }

    LOG(LOG_INFO) << "three.js file: " << filename;
    LOG(LOG_INFO) << "just open it in a browser";

    oss << header;                  // header
    oss << buffer.str();            // content
    oss << footer;                  // footer
    oss << std::endl;
}
