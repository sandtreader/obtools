#include "ot-netlink.h"
#include <gtest/gtest.h>
#include <linux/genetlink.h>

namespace {

using namespace std;
using namespace ObTools;
using namespace ObTools::Netlink;

// Use the generic netlink family controller for testing
const string family("nlctrl");

static struct nla_policy policy[] =
{
  { NLA_UNSPEC, 0, 0 },
  { NLA_U16, 0, 0 },              // id
  { NLA_STRING, 0, GENL_NAMSIZ }, // name
  { NLA_U32, 0, 0 },              // version
  { NLA_U32, 0, 0 },              // header size
  { NLA_U32, 0, 0 },              // max attr
  { NLA_NESTED, 0, 0},            // ops
};

static struct nla_policy op_policy[] =
{
  { NLA_UNSPEC, 0, 0 },
  { NLA_U32, 0, 0 },              // id
  { NLA_U32, 0, 0 },              // flags
};

class GenericNetlinkTest: public ::testing::Test
{
protected:

  virtual void SetUp()
  {
  }

  virtual void TearDown()
  {
  }

public:
};

class GenericTestRequest: public GenericRequest
{
public:
  string name;
  uint32_t max_attr;

  GenericTestRequest(const GenericNetlink& netlink, const string& family):
    GenericRequest(netlink, CTRL_CMD_GETFAMILY, 0x0001)
  {
    set_string(CTRL_ATTR_FAMILY_NAME, family);
  }

  virtual int get_attribute_count() { return CTRL_ATTR_MAX; }

  virtual struct nla_policy *get_policy() { return policy; }

  virtual int callback(GenericResponse& response)
  {
    name = response.get_string(CTRL_ATTR_FAMILY_NAME);
    max_attr = response.get_uint32(CTRL_ATTR_MAXATTR);
    return 0;
  }

  virtual ~GenericTestRequest() {}
};

class GenericNestedTestRequest: public GenericTestRequest
{
public:
  uint32_t op_id;

  GenericNestedTestRequest(const GenericNetlink& netlink,
                           const string& family):
    GenericTestRequest(netlink, family)
  {}

  virtual int callback(GenericResponse& response)
  {
    int result = GenericTestRequest::callback(response);
    if (result)
      return result;
    vector<struct nlattr *> nested;
    response.get_nested_attrs(CTRL_ATTR_OPS, nested,
                              CTRL_ATTR_OP_MAX, op_policy);
    op_id = response.get_uint32(CTRL_ATTR_OP_ID, nested);
    return 0;
  }

  virtual ~GenericNestedTestRequest() {}
};

TEST_F(GenericNetlinkTest, TestGettingAFamilyLink)
{
  GenericNetlink genl(family);
  ASSERT_TRUE(genl.valid()) << genl.get_last_error();
}

TEST_F(GenericNetlinkTest, TestGettingABasicResponse)
{
  GenericNetlink genl(family);
  ASSERT_TRUE(genl.valid()) << genl.get_last_error();
  const string name("nlctrl");
  GenericTestRequest request(genl, name);
  ASSERT_TRUE(genl.send(request)) << genl.get_last_error();
  ASSERT_EQ(name, request.name);
  ASSERT_EQ(CTRL_ATTR_MAX, request.max_attr);
}

TEST_F(GenericNetlinkTest, TestGettingANestedResponse)
{
  GenericNetlink genl(family);
  ASSERT_TRUE(genl.valid()) << genl.get_last_error();
  const string name("nlctrl");
  GenericNestedTestRequest request(genl, name);
  ASSERT_TRUE(genl.send(request)) << genl.get_last_error();
  ASSERT_EQ(name, request.name);
  ASSERT_EQ(CTRL_ATTR_MAX, request.max_attr);
  ASSERT_EQ(65544, request.op_id);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
