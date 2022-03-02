#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr double MY_PI = 3.1415926;

inline double DEG2RAD(double deg){return deg * MY_PI / 180.0;}

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.
    Eigen::Matrix4f translate;

    double rad = DEG2RAD(rotation_angle);

    translate << cos(rad), -sin(rad), 0, 0,
    sin(rad), cos(rad), 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1;

    model = translate * model;

    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.
    Eigen::Matrix4f ortho_scale;
    Eigen::Matrix4f ortho_trans;
    Eigen::Matrix4f persp2ortho;
    
    double rad = DEG2RAD(eye_fov) / 2.0;
    double t = -1.0 * zNear * tan(rad);
    double r = -1.0 * t * aspect_ratio;
    double l = -r;
    double b = -t;

    ortho_scale << 2.0 / (r - l), 0, 0, 0,
    0, 2.0 / (t - b), 0, 0,
    0, 0, 2.0 / (zNear - zFar), 0,
    0, 0, 0, 1;

    ortho_trans << 1, 0, 0, -1.0 * (r + l) / 2.0,
    0, 1, 0, -1.0 * (t + b) / 2.0,
    0, 0, 1, -1.0 * (zNear + zFar) / 2.0,
    0, 0, 0, 1;

    persp2ortho << zNear, 0, 0, 0,
    0, zNear, 0, 0,
    0, 0, zNear + zFar, -1.0 * zNear * zFar,
    0, 0, 1, 0;

    projection = ortho_scale * ortho_trans * persp2ortho;

    return projection;
}

Eigen::Matrix4f get_rotation(Vector3f axis, float angle)
{
    Eigen::Matrix4f rotation = Eigen::Matrix4f::Identity();
    Eigen::Matrix3f I = Eigen::Matrix3f::Identity();
    Eigen::Matrix3f R;
    Eigen::Matrix3f N;

    double rad = DEG2RAD(angle);

    /* for (auto x: axis)
        std::cout << x << " "; */
    
    N << 0, -axis[2], axis[1],
    axis[2], 0, -axis[0],
    -axis[1], axis[0], 0;

    R = cos(rad) * I + (1 - cos(rad)) * axis * axis.transpose() + sin(rad) * N;

    rotation << R(0, 0), R(0, 1), R(0, 2), 0,
    R(1, 0), R(1, 1), R(1, 2), 0,
    R(2, 0), R(2, 1), R(2, 2), 0,
    0, 0, 0, 1;

    return rotation;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    bool any_axis = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
        else
            return 0;
    }
    else if (argc == 2) {
        any_axis = true;
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    Eigen::Vector3f axis = {1, 0, 1};

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        if (!any_axis)
            r.set_model(get_model_matrix(angle));
        else
            r.set_model(get_rotation(axis, angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}