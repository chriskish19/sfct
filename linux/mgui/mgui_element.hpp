#pragma once
#include <cstdint>
#include <memory>

namespace mgui{
    enum class element_state{

    };

    class element{
    public:
        element(std::int32_t width,std::int32_t height);

    protected:
        std::shared_ptr<std::int32_t> m_width{std::make_shared<std::int32_t>(0)};
        std::shared_ptr<std::int32_t> m_height{std::make_shared<std::int32_t>(0)};
        std::shared_ptr<std::int32_t> m_x_pos{std::make_shared<std::int32_t>(0)};
        std::shared_ptr<std::int32_t> m_y_pos{std::make_shared<std::int32_t>(0)};
    };
}