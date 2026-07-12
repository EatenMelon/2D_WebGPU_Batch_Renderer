#include "Material.h"

wgpu::Material::Material(const Pipeline& pipeline)
	: m_Pipeline{ &pipeline }
{}
