#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <chrono>
#include <thread>
#include <algorithm>
#include <limits>
#include <cstdlib>
#include <ctime>

constexpr double PI = 3.14159265358979323846;

// ANSI Escape Codes
namespace Color {
    const std::string RESET = "\033[0m";
    const std::string BLACK = "\033[30m";
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN = "\033[36m";
    const std::string WHITE = "\033[37m";
    const std::string BRIGHT_BLUE = "\033[94m";
    const std::string BRIGHT_GREEN = "\033[92m";
    const std::string BRIGHT_RED = "\033[91m";
    const std::string BRIGHT_YELLOW = "\033[93m";
    const std::string BRIGHT_MAGENTA = "\033[95m";
    const std::string BRIGHT_CYAN = "\033[96m";
    const std::string BRIGHT_WHITE = "\033[97m";
}

// 3D Vector operations
struct Vec3 {
    double x, y, z;

    Vec3(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}

    // Operators (+/-/*)
    Vec3 operator+(const Vec3& v) const {
        return Vec3(x + v.x, y + v.y, z + v.z);
    }

    Vec3 operator-(const Vec3& v) const {
        return Vec3(x - v.x, y - v.y, z - v.z);
    }

    Vec3 operator*(double s) const {
        return Vec3(x * s, y * s, z * s);
    }

    // dot Product
    double dot(const Vec3& v) const {
        return x * v.x + y * v.y + z * v.z;
    }

    // Length & Magnitude
    double length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    // Be normal!!!
    Vec3 normalize() const {
        double len = length();
        if (len < 1e-10) return Vec3();
        return Vec3(x / len, y / len, z / len);
    }

    // Cross Product
    Vec3 cross(const Vec3& v) const {
        return Vec3(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        );
    }
};

// I used Rodrigues' Rotation Formula (Axis-Angle Rotation) See readme or google it!
Vec3 rotate(const Vec3& v, const Vec3& axis, double angle) {
    double c = std::cos(angle);
    double s = std::sin(angle);
    double k = 1.0 - c;

    Vec3 a = axis.normalize();
    double ax = a.x, ay = a.y, az = a.z;

    double rotMatrix[3][3] = {
        {c + k * ax * ax, k * ax * ay - s * az, k * ax * az + s * ay},
        {k * ay * ax + s * az, c + k * ay * ay, k * ay * az - s * ax},
        {k * az * ax - s * ay, k * az * ay + s * ax, c + k * az * az}
    };

    return Vec3(
        v.x * rotMatrix[0][0] + v.y * rotMatrix[0][1] + v.z * rotMatrix[0][2],
        v.x * rotMatrix[1][0] + v.y * rotMatrix[1][1] + v.z * rotMatrix[1][2],
        v.x * rotMatrix[2][0] + v.y * rotMatrix[2][1] + v.z * rotMatrix[2][2]
    );
}

// Camera [o]
class Camera {
public:
    Vec3 position;
    Vec3 lookAt;
    Vec3 up;
    double fov;
    double aspectRatio;

    Camera(const Vec3& pos, const Vec3& look, const Vec3& u, double f, double ar)
        : position(pos), lookAt(look), up(u), fov(f), aspectRatio(ar) {}

    // Ray Direction/Persp Projection
    Vec3 rayDirection(double screenX, double screenY) const {
        Vec3 forward = (lookAt - position).normalize();
        Vec3 right = forward.cross(up).normalize();
        Vec3 trueUp = right.cross(forward);

        double fovRadians = fov * PI / 180.0;
        double heightAtDist1 = 2.0 * std::tan(fovRadians / 2.0);
        double widthAtDist1 = heightAtDist1 * aspectRatio;

        Vec3 dir = forward
                + right * (screenX * widthAtDist1)
                + trueUp * (screenY * heightAtDist1);

        return dir.normalize();
    }
};

// Da Sphere (Earth)
class Earth {
public:
    double radius;
    Vec3 position;
    double rotationY;
    std::vector<std::vector<bool>> texture;

    Earth(double r, const Vec3& pos)
        : radius(r), position(pos), rotationY(0.0) {
        createSimplifiedTexture();
    }

    // Proc Texture Gen, Noise Pattern using sin/cos
    void createSimplifiedTexture() {
        texture.resize(180);
        for (auto& row : texture) {
            row.resize(360, false);
        }
        std::srand(42);

        for (int i = 0; i < 5; i++) {
            int centerLat = 30 + std::rand() % 120;
            int centerLon = std::rand() % 360;
            int size = 15 + std::rand() % 20;
            for (int lat = centerLat - size; lat < centerLat + size; lat++) {
                if (lat < 0 || lat >= 180) continue;
                for (int lon = centerLon - size; lon < centerLon + size; lon++) {
                    int wrappedLon = (lon + 360) % 360;
                    double latDist = (lat - centerLat) / static_cast<double>(size);
                    double lonDist = (lon - centerLon) / static_cast<double>(size);
                    double distance = std::sqrt(latDist * latDist + lonDist * lonDist);
                    double noise = 0.3 * std::sin(lat * 0.1) * std::cos(lon * 0.1);
                    noise += 0.2 * std::sin(lat * 0.2 + 0.5) * std::cos(lon * 0.2 + 0.3);
                    noise += 0.1 * std::sin(lat * 0.4 + 1.0) * std::cos(lon * 0.4 + 0.7);
                    if (distance < 0.8 + noise) texture[lat][wrappedLon] = true;
                }
            }
        }
         for (int i = 0; i < 12; i++) {
            int centerLat = 20 + std::rand() % 140;
            int centerLon = std::rand() % 360;
            int size = 3 + std::rand() % 5;
            for (int lat = centerLat - size; lat < centerLat + size; lat++) {
                if (lat < 0 || lat >= 180) continue;
                for (int lon = centerLon - size; lon < centerLon + size; lon++) {
                    int wrappedLon = (lon + 360) % 360;
                    double latDist = (lat - centerLat) / static_cast<double>(size);
                    double lonDist = (lon - centerLon) / static_cast<double>(size);
                    double distance = std::sqrt(latDist * latDist + lonDist * lonDist);
                    double noise = 0.2 * std::sin(lat * 0.3) * std::cos(lon * 0.3);
                    if (distance < 0.7 + noise) texture[lat][wrappedLon] = true;
                }
            }
        }
        for (int lat = 0; lat < 20; lat++) {
            for (int lon = 0; lon < 360; lon++) {
                double noise = 0.2 * std::sin(lon * 0.1);
                if (lat < 15 + noise * 5) texture[lat][lon] = true;
            }
        }
        for (int lat = 160; lat < 180; lat++) {
            for (int lon = 0; lon < 360; lon++) {
                double noise = 0.2 * std::sin(lon * 0.1);
                if (lat > 165 - noise * 5) texture[lat][lon] = true;
            }
        }
        for (int i = 0; i < 25; i++) {
            int centerLat = 30 + std::rand() % 120;
            int centerLon = std::rand() % 360;
            int size = 2 + std::rand() % 8;
            if (centerLat < 0 || centerLat >= 180 || centerLon < 0 || centerLon >= 360 || !texture[centerLat][centerLon]) continue;
            for (int lat = centerLat - size; lat < centerLat + size; lat++) {
                if (lat < 0 || lat >= 180) continue;
                for (int lon = centerLon - size; lon < centerLon + size; lon++) {
                    int wrappedLon = (lon + 360) % 360;
                    double latDist = (lat - centerLat) / static_cast<double>(size);
                    double lonDist = (lon - centerLon) / static_cast<double>(size);
                    double distance = std::sqrt(latDist * latDist + lonDist * lonDist);
                    if (distance < 0.8) texture[lat][wrappedLon] = false;
                }
            }
        }
    }

    // texture Coordinate Mapping
    bool isLand(double lat, double lon) const {
        int latIdx = static_cast<int>((lat + 90.0) / 180.0 * texture.size());
        int lonIdx = static_cast<int>((lon + 180.0) / 360.0 * texture[0].size());
        latIdx = std::max(0, std::min(static_cast<int>(texture.size() - 1), latIdx));
        lonIdx = std::max(0, std::min(static_cast<int>(texture[0].size() - 1), lonIdx));
        return texture[latIdx][lonIdx];
    }

    void rotate(double angleDegrees) {
        rotationY += angleDegrees * PI / 180.0;
        while (rotationY >= 2.0 * PI) rotationY -= 2.0 * PI;
        while (rotationY < 0.0) rotationY += 2.0 * PI;
    }

    // Ray Sphere Intersect
    bool intersectRay(const Vec3& rayOrigin, const Vec3& rayDir, double& depth, Vec3& hitPoint, Vec3& normal) const {
        Vec3 oc = rayOrigin - position;
        double a = rayDir.dot(rayDir);
        double b = 2.0 * oc.dot(rayDir);
        double c = oc.dot(oc) - radius * radius;
        double discriminant = b * b - 4 * a * c;

        if (discriminant < 0) {
            return false;
        }

        double sqrtDiscriminant = std::sqrt(discriminant);
        double t1 = (-b - sqrtDiscriminant) / (2.0 * a);
        double t2 = (-b + sqrtDiscriminant) / (2.0 * a);

        double t = -1.0;
        if (t1 >= 0) {
            t = t1;
        }
        if (t2 >= 0 && (t < 0 || t2 < t)) {
             t = t2;
        }

        if (t < 0) {
            return false;
        }


        depth = t;
        hitPoint = rayOrigin + rayDir * t;
        normal = (hitPoint - position).normalize();
        return true;
    }

    // ASCIIIIIII
    char getTextureChar(const Vec3& hitPoint) const {
        Vec3 rotatedPoint = ::rotate(hitPoint - position, Vec3(0, 1, 0), -rotationY);
        Vec3 dirFromCenter = (rotatedPoint).normalize();
        // Cartesian to Spherical
        double lat = std::asin(dirFromCenter.y) * 180.0 / PI;
        double lon = std::atan2(dirFromCenter.z, dirFromCenter.x) * 180.0 / PI;

        if (isLand(lat, lon)) {
            return '#';
        } else {
            return '~';
        }
    }
};

// ASCIIIIIIIIII
class ASCIIRenderer {
private:
    int width, height;
    std::vector<std::vector<char>> frameBuffer;
    std::vector<std::vector<std::string>> colorBuffer;
    std::vector<std::vector<double>> depthBuffer;
    Camera camera;
    bool useColor;

public:
    // Why characters gotta be so weird, aspect ratio took a while to get right.
    ASCIIRenderer(int w, int h, bool color = true)
        : width(w),
          height(h),
          useColor(color),
          camera(Vec3(0, 0, -8), Vec3(0, 0, 0), Vec3(0, 1, 0), 45.0, static_cast<double>(w) / h * 0.4) {

        frameBuffer.resize(height, std::vector<char>(width, ' '));
        colorBuffer.resize(height, std::vector<std::string>(width, Color::RESET));
        depthBuffer.resize(height, std::vector<double>(width, std::numeric_limits<double>::max()));
    }

    void clearBuffers() {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                frameBuffer[y][x] = ' ';
                colorBuffer[y][x] = Color::RESET;
                depthBuffer[y][x] = std::numeric_limits<double>::max();
            }
        }
    }

    // Starfield
    void renderStars() {
        for (int i = 0; i < width * height / 100; i++) {
            int x = std::rand() % width;
            int y = std::rand() % height;
            if (depthBuffer[y][x] == std::numeric_limits<double>::max()) {
                frameBuffer[y][x] = (std::rand() % 10 == 0) ? '+' : '.';
                colorBuffer[y][x] = Color::WHITE;
            }
        }
    }

    // Ray Casting Rendering Pipeline
    void render(const Earth& earth) {
        clearBuffers();

        Vec3 lightDir = Vec3(std::cos(earth.rotationY), 0.5, -std::sin(earth.rotationY)).normalize();
        double cloudPhase = earth.rotationY * 0.7;

        renderStars();

        // Pixel Iteration
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                double screenX = 2.0 * (static_cast<double>(x) / width) - 1.0;
                double screenY = 1.0 - 2.0 * (static_cast<double>(y) / height);
                Vec3 rayDir = camera.rayDirection(screenX, screenY);

                double depth;
                Vec3 hitPoint, normal;

                if (earth.intersectRay(camera.position, rayDir, depth, hitPoint, normal)) {
                    // Depth
                    if (depth < depthBuffer[y][x]) {
                        // this is where we use Dot Product (:
                        double diffuse = std::max(0.0, normal.dot(lightDir));
                        char texChar = earth.getTextureChar(hitPoint);

                        Vec3 rotatedPoint = ::rotate(hitPoint - earth.position, Vec3(0, 1, 0), -earth.rotationY);
                        Vec3 dirFromCenter = (rotatedPoint).normalize();
                        double lat_rad = std::asin(dirFromCenter.y);
                        double lon_rad = std::atan2(dirFromCenter.z, dirFromCenter.x);

                        if (useColor) {
                            if (texChar == '#') {
                                double polarFactor = std::abs(lat_rad / (PI / 2.0));
                                colorBuffer[y][x] = (polarFactor > 0.7) ? Color::BRIGHT_WHITE : Color::BRIGHT_GREEN;
                                if (diffuse > 0.8) frameBuffer[y][x] = '%';
                                else if (diffuse > 0.6) frameBuffer[y][x] = '&';
                                else if (diffuse > 0.3) frameBuffer[y][x] = '$';
                                else frameBuffer[y][x] = '#';
                            } else {
                                colorBuffer[y][x] = diffuse > 0.7 ? Color::BRIGHT_BLUE : Color::BLUE;
                                if (diffuse > 0.8) frameBuffer[y][x] = '~';
                                else if (diffuse > 0.6) frameBuffer[y][x] = '^';
                                else frameBuffer[y][x] = '.';
                            }

                            if (diffuse >= 0.2) {
                                // Clouds
                                double noise1 = std::sin(lat_rad * 8.0 + cloudPhase * 0.5) * std::cos(lon_rad * 6.0 + cloudPhase * 0.4);
                                double noise2 = std::sin(lat_rad * 18.0 - cloudPhase * 0.8) * std::cos(lon_rad * 14.0 - cloudPhase * 0.6);
                                double noise3 = std::sin(lat_rad * 30.0 + cloudPhase * 1.2) * std::cos(lon_rad * 25.0 + cloudPhase * 1.0);
                                double cloudValue = 0.4 * noise1 + 0.3 * noise2 + 0.3 * noise3;
                                cloudValue = (cloudValue + 0.3);
                                cloudValue = std::max(0.0, cloudValue - 0.6) * 2.0;

                                if (cloudValue > 0.1) {
                                    colorBuffer[y][x] = Color::BRIGHT_WHITE;
                                    if (cloudValue > 0.7) frameBuffer[y][x] = '@';
                                    else if (cloudValue > 0.3) frameBuffer[y][x] = '%';
                                    else frameBuffer[y][x] = '.';
                                }
                            }

                            // night
                            if (diffuse < 0.2) {
                                if (texChar == '#') {
                                    colorBuffer[y][x] = Color::BLACK;
                                    frameBuffer[y][x] = '.';
                                    if (std::rand() % 25 == 0) {
                                        colorBuffer[y][x] = Color::BRIGHT_YELLOW;
                                    }
                                } else {
                                    colorBuffer[y][x] = Color::BLUE;
                                    frameBuffer[y][x] = ' ';
                                }
                            }
                        }
                        else {
                             if (texChar == '#') {
                                if (diffuse > 0.8) frameBuffer[y][x] = '%';
                                else if (diffuse > 0.6) frameBuffer[y][x] = '&';
                                else if (diffuse > 0.3) frameBuffer[y][x] = '$';
                                else if (diffuse >= 0.2) frameBuffer[y][x] = '#';
                                else frameBuffer[y][x] = '.';
                            } else {
                                if (diffuse > 0.8) frameBuffer[y][x] = '~';
                                else if (diffuse > 0.6) frameBuffer[y][x] = '^';
                                else if (diffuse >= 0.2) frameBuffer[y][x] = '.';
                                else frameBuffer[y][x] = ' ';
                            }
                             if (diffuse >= 0.2) {
                                double noise1 = std::sin(lat_rad * 8.0 + cloudPhase * 0.5) * std::cos(lon_rad * 6.0 + cloudPhase * 0.4);
                                double noise2 = std::sin(lat_rad * 18.0 - cloudPhase * 0.8) * std::cos(lon_rad * 14.0 - cloudPhase * 0.6);
                                double noise3 = std::sin(lat_rad * 30.0 + cloudPhase * 1.2) * std::cos(lon_rad * 25.0 + cloudPhase * 1.0);
                                double cloudValue = 0.4 * noise1 + 0.3 * noise2 + 0.3 * noise3;
                                cloudValue = (cloudValue + 0.3);
                                cloudValue = std::max(0.0, cloudValue - 0.6) * 2.0;
                                if (cloudValue > 0.1) {
                                    if (cloudValue > 0.7) frameBuffer[y][x] = '@';
                                    else if (cloudValue > 0.3) frameBuffer[y][x] = '%';
                                    else frameBuffer[y][x] = '.';
                                }
                            }
                        }
                        depthBuffer[y][x] = depth;
                    }
                }
            }
        }
    }

    // Terminal out
    void display() const {
        // Hopefully this covers my bases? I tried on both Windows and Ubuntu on WSL and it seemed to work fine.
        #ifdef _WIN32
        system("cls");
        #else
        system("clear");
        #endif

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                std::cout << colorBuffer[y][x] << frameBuffer[y][x] << (useColor ? Color::RESET : "");
            }
            std::cout << std::endl;
        }

        // Methodology: ASCII Art Rendering
        if (useColor) std::cout << Color::BRIGHT_CYAN;
        std::string padding((width - 58) / 2, ' ');
        std::cout << padding << " _   _      _ _                            _     _ _ \n";
        std::cout << padding << "| | | | ___| | | ___   __      _____  _ __| | __| | |\n";
        std::cout << padding << "| |_| |/ _ \\ | |/ _ \\  \\ \\ /\\ / / _ \\| '__| |/ _` | |\n";
        std::cout << padding << "|  _  |  __/ | | (_) |  \\ V  V / (_) | |  | | (_| |_|\n";
        std::cout << padding << "|_| |_|\\___|_|_|\\___/    \\_/\\_/ \\___/|_|  |_|\\__,_(_)\n";
        if (useColor) std::cout << Color::RESET;
        std::cout << std::endl;
    }
};

int main() {
    const int width = 150;
    const int height = 50;
    bool useColor = true;

    std::cout << "ASCII Earth 3D Renderer" << std::endl;
    std::cout << "========================" << std::endl;
    std::cout << "Press Ctrl+C to exit" << std::endl;
    std::cout << "% & $ # : Land (daytime lighting)" << std::endl;
    std::cout << ". : Land (nighttime) / Wispy Clouds (daytime) / Stars" << std::endl;
    std::cout << "~ ^ : Ocean (daytime lighting)" << std::endl;
    std::cout << "' ' : Ocean (nighttime)" << std::endl;
    std::cout << "@ % : Dense/Medium Clouds (daytime)" << std::endl;
    std::cout << "+ : Bright Stars" << std::endl;
    std::cout << Color::BRIGHT_YELLOW << "." << Color::RESET << " : City Lights (nighttime)" << std::endl;
    std::cout << std::endl;

    ASCIIRenderer renderer(width, height, useColor);
    Earth earth(3.0, Vec3(0, 0, 0));

    // the seed
    std::srand(42);

    double rotationSpeed = 0.03;

    while (true) {
        renderer.render(earth);
        renderer.display();
        earth.rotationY += rotationSpeed;
        if (earth.rotationY >= 2.0 * PI) earth.rotationY -= 2.0 * PI;
        // Im controlling the frames with time because I am lazy.
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}