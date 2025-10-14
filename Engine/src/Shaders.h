#pragma once

class Shader
{
public:
    Shader();
    ~Shader();

    bool Create();
    void Use() const;
    void Delete();

    unsigned int GetProgramID() const { return shaderProgram; }

private:
    unsigned int shaderProgram;
};