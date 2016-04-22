#pragma once

struct CoordinatePair {
    double x;
    double y;

    CoordinatePair() {
        x = 0;
        y = 0;
    }

    CoordinatePair(const CoordinatePair& other) {
        x = other.x;
        y = other.y;
    }

    template<typename T>
    CoordinatePair(const sf::Vector2<T>& other) {
        x = other.x;
        y = other.y;
    }

    template<typename T>
    CoordinatePair(const T& ox, const T& oy) {
        x = ox;
        y = oy;
    }

    template<typename T>
    CoordinatePair& add(const T& ox, const T& oy) {
        x += ox;
        y += oy;

        return *this;
    }

    template<typename T>
    CoordinatePair& subtract(const T& ox, const T& oy) {
        return add(-ox, -oy);
    }

    template<typename T>
    CoordinatePair& multiply(const T& ox, const T& oy) {
        x *= ox;
        y *= oy;

        return *this;
    }

    template<typename T>
    CoordinatePair& multiply(const T& v) {
        return multiply(v, v);
    }

    CoordinatePair& add(const CoordinatePair& other) {
        return add(other.x, other.y);
    }

    CoordinatePair& subtract(const CoordinatePair& other) {
        return add(-other.x, -other.y);
    }

    CoordinatePair& multiply(const CoordinatePair& other) {
        return multiply(other.x, other.y);
    }

    template<typename T>
    CoordinatePair& add(const sf::Vector2<T>& other) {
        return add(other.x, other.y);
    }

    template<typename T>
    CoordinatePair& subtract(const sf::Vector2<T>& other) {
        return add(-other.x, -other.y);
    }

    template<typename T>
    CoordinatePair& multiply(const sf::Vector2<T>& other) {
        return multiply(other.x, other.y);
    }

    CoordinatePair operator+(const CoordinatePair& other) {
        return CoordinatePair(*this).add(other);
    }

    CoordinatePair operator-(const CoordinatePair& other) {
        return CoordinatePair(*this).subtract(other);
    }

    CoordinatePair operator*(const CoordinatePair& other) {
        return CoordinatePair(*this).multiply(other);
    }

    template<typename T>
    CoordinatePair operator+(const sf::Vector2<T>& other) {
        return CoordinatePair(*this).add(other);
    }

    template<typename T>
    CoordinatePair operator-(const sf::Vector2<T>& other) {
        return CoordinatePair(*this).subtract(other);
    }

    template<typename T>
    CoordinatePair operator*(const sf::Vector2<T>& other) {
        return CoordinatePair(*this).multiply(other);
    }

    template<typename T>
    CoordinatePair operator*(const T& v) {
        return CoordinatePair(*this).multiply(v);
    }

    CoordinatePair operator-() {
        return { -x, -y };
    }

    sf::Vector2f toSfVector2f() {
        return sf::Vector2f((float)x, (float)y);
    }
};
